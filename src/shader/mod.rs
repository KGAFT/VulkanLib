#![allow(static_mut_refs)]

use crate::pipelines::shader::VlShader;
use crate::util::vl_semaphore::VlSemaphore;
use ash::vk;

use shaderc::{
    CompileOptions, Compiler, OptimizationLevel, ResolvedInclude, ShaderKind, SourceLanguage,
    SpirvVersion,
};
use std::ffi::CString;
use std::fs;
use std::fs::File;
use std::io::Read;
use std::path::{Path, PathBuf};
use std::sync::mpsc;
use std::sync::mpsc::Receiver;
use threadpool::ThreadPool;

#[derive(Default)]
pub enum VlShaderFileType {
    SrcFile,
    #[default]
    BinFile,
}

#[derive(Default)]
pub struct VlShaderCreateInfo {
    pub path: PathBuf,
    pub file_type: VlShaderFileType,
    pub stage: vk::ShaderStageFlags,
    pub entry_point: String,
}

static mut INCLUDE_DIRECTORIES: Option<Vec<String>> = None;
static mut INCLUDE_SEM: VlSemaphore = VlSemaphore::new(1);
pub struct VlShaderLoader {
    compiler: Compiler,
    compile_options_stub: CompileOptionsStub,
    thread_pool: ThreadPool,
}

impl VlShaderLoader {
    pub fn add_include_directory(directory: &Path) {
        unsafe {
            if INCLUDE_DIRECTORIES.is_none() {
                INCLUDE_DIRECTORIES = Some(Vec::new());
            }
            INCLUDE_SEM.acquire();
            let mut path = directory
                .canonicalize()
                .unwrap()
                .to_str()
                .unwrap()
                .to_string();
            if !path.ends_with("/") && !path.ends_with("\\") {
                path += "/"
            }

            INCLUDE_DIRECTORIES.as_mut().unwrap().push(path);
            INCLUDE_SEM.release();
        }
    }

    pub fn new() -> Self {
        Self {
            compiler: Compiler::new().unwrap(),
            thread_pool: ThreadPool::new(std::thread::available_parallelism().unwrap().get()),
            compile_options_stub: Default::default(),
        }
    }

    pub fn new_n_threads(threads_amount: usize) -> Self {
        Self {
            compiler: Compiler::new().unwrap(),
            compile_options_stub: Default::default(),
            thread_pool: ThreadPool::new(threads_amount),
        }
    }

    pub fn set_optimization_level(&mut self, level: OptimizationLevel) {
        self.compile_options_stub.optimization_level = level;
    }

    pub fn set_source_language(&mut self, src_language: SourceLanguage) {
        self.compile_options_stub.src_language = src_language;
    }

    pub fn set_debug_info(&mut self, debug: bool) {
        self.compile_options_stub.debug_info = debug;
    }

    pub fn create_shader(
        &self,
        device: &ash::Device,
        create_info: &Vec<VlShaderCreateInfo>,
    ) -> Option<VlShader> {
        let mut infos: Vec<vk::PipelineShaderStageCreateInfo> =
            Vec::with_capacity(create_info.len());
        let mut names: Vec<CString> = Vec::with_capacity(create_info.len());
        create_info.iter().for_each(|info| {
            names.push(CString::new(info.entry_point.clone()).unwrap());
        });
        let mut failed: bool = false;
        let mut i = 0;
        for x in create_info.iter() {
            let module = self.create_vulkan_shader_module(device, x);
            if module.is_none() {
                failed = true;
                break;
            }
            let module = module.unwrap();
            let entry_name = names[i].as_c_str();
            let info = vk::PipelineShaderStageCreateInfo {
                stage: x.stage,
                module,
                ..Default::default()
            }
            .name(entry_name);

            infos.push(info);
            i+=1;
        }
        if failed {
            for x in infos.iter() {
                unsafe {
                    device.destroy_shader_module(x.module, None);
                }
            }
            return None;
        }
        return Some(VlShader::new(infos, device.clone()));
    }

    pub fn create_vulkan_shader_module(
        &self,
        device: &ash::Device,
        create_info: &VlShaderCreateInfo,
    ) -> Option<vk::ShaderModule> {
        let binary = match create_info.file_type {
            VlShaderFileType::SrcFile => self.read_and_compile_shader(
                create_info.path.as_path(),
                vk_stage_to_shaderc_kind(create_info.stage).unwrap(),
                create_info.entry_point.as_str(),
            ),
            VlShaderFileType::BinFile => Self::read_spv_shader(create_info.path.as_path()),
        };
        if binary.is_none() {
            return None;
        }
        let binary = binary.unwrap();
        Self::create_vulkan_shader_module_int(device, binary)
    }

    fn create_vulkan_shader_module_int(
        device: &ash::Device,
        binary: Vec<u32>,
    ) -> Option<vk::ShaderModule> {
        let shader_create_info = vk::ShaderModuleCreateInfo {
            ..Default::default()
        }
        .code(binary.as_slice());
        let result = unsafe { device.create_shader_module(&shader_create_info, None) };
        if result.is_err() {
            eprintln!(
                "Failed to create shader module: {}",
                result.unwrap_err().to_string()
            );
        }
        Some(result.unwrap())
    }

    pub fn read_spv_shader<P: AsRef<Path>>(path: P) -> Option<Vec<u32>> {
        let file = File::open(&path);
        if file.is_err() {
            eprintln!(
                "Failed to open shader spv shader: {}",
                path.as_ref().display()
            );
            return None;
        }
        let mut file = file.unwrap();
        let mut bytes = Vec::new();
        let res = file.read_to_end(&mut bytes);

        if res.is_err() {
            eprintln!("Failed to read shader: {}", path.as_ref().display());
            return None;
        }

        if bytes.len() % 4 != 0 {
            eprintln!("spv file alignment failed: {}", path.as_ref().display());
            return None;
        }
        let words: Vec<u32> = bytes
            .chunks_exact(4)
            .map(|chunk| u32::from_le_bytes([chunk[0], chunk[1], chunk[2], chunk[3]]))
            .collect();

        Some(words)
    }

    pub fn read_and_compile_shader(
        &self,
        path: &Path,
        kind: ShaderKind,
        entry_point_name: &str,
    ) -> Option<Vec<u32>> {
        let content = Self::read_content_int(path);
        if content.is_none() {
            eprintln!("Failed to read shader: {}", path.display());
            return None;
        }
        let content = content.unwrap();
        let res = self.compile_shader(
            content.as_str(),
            kind,
            path.file_name().unwrap().to_str()?,
            entry_point_name,
        );
        if res.is_err() {
            eprintln!(
                "Failed to compile shader: {} {}",
                path.display(),
                res.err().unwrap()
            );
            return None;
        }
        return Some(res.unwrap());
    }

    pub fn read_and_compile_shader_parallel(
        &self,
        path: &Path,
        kind: ShaderKind,
        entry_point_name: &str,
    ) -> Option<Receiver<shaderc::Result<Vec<u32>>>> {
        let content = Self::read_content_int(path);
        if content.is_none() {
            return None;
        }
        let content = content.unwrap();
        let res = self.compile_shader_parallel(
            content,
            kind,
            path.file_name().unwrap().to_str()?,
            entry_point_name,
        );
        Some(res)
    }

    fn read_content_int(path: &Path) -> Option<String> {
        let content = fs::read_to_string(path);
        if content.is_err() {
            eprintln!("Failed to read shader: {}", path.display());
            return None;
        }
        return Some(content.unwrap());
    }

    pub fn compile_shader(
        &self,
        source: &str,
        kind: ShaderKind,
        input_file_name: &str,
        entry_point_name: &str,
    ) -> shaderc::Result<Vec<u32>> {
        let compile_options = self.compile_options_stub.get_compile_options();
        Self::compile_shader_int(
            &self.compiler,
            source,
            kind,
            input_file_name,
            entry_point_name,
            &compile_options,
        )
    }

    fn compile_shader_int(
        compiler: &Compiler,
        source: &str,
        kind: ShaderKind,
        input_file_name: &str,
        entry_point_name: &str,
        compile_options: &CompileOptions,
    ) -> shaderc::Result<Vec<u32>> {
        let res = compiler.compile_into_spirv(
            source,
            kind,
            input_file_name,
            entry_point_name,
            Some(compile_options),
        );
        let new_res: shaderc::Result<Vec<u32>> = if res.is_err() {
            Err(res.err().unwrap())
        } else {
            Ok(Vec::from(res.unwrap().as_binary()))
        };
        new_res
    }

    pub fn compile_shader_parallel(
        &self,
        source: String,
        kind: ShaderKind,
        input_file_name: &str,
        entry_point_name: &str,
    ) -> Receiver<shaderc::Result<Vec<u32>>> {
        let (tx, rx) = mpsc::channel();
        let input_file_name = String::from(input_file_name);
        let entry_point_name = String::from(entry_point_name);
        let compiler = Compiler::new().unwrap();
        let compile_options = self.compile_options_stub.clone();
        self.thread_pool.execute(move || {
            let compile_options2 = compile_options.get_compile_options();
            tx.send(Self::compile_shader_int(
                &compiler,
                source.as_str(),
                kind,
                input_file_name.as_str(),
                entry_point_name.as_str(),
                &compile_options2,
            ))
            .unwrap();
        });
        rx
    }
}
#[derive(Copy, Clone)]
struct CompileOptionsStub {
    pub optimization_level: OptimizationLevel,
    pub debug_info: bool,
    pub src_language: SourceLanguage,
}

impl CompileOptionsStub {
    fn get_compile_options(&self) -> CompileOptions<'_> {
        let mut compile_options = CompileOptions::new().unwrap();
        compile_options.set_optimization_level(self.optimization_level);
        compile_options.set_include_callback(include_callback);
        if self.debug_info {
            compile_options.set_generate_debug_info();
        }
        compile_options.set_source_language(self.src_language);
        compile_options.set_target_spirv(SpirvVersion::V1_6);
        compile_options
    }
}

impl Default for CompileOptionsStub {
    fn default() -> Self {
        Self {
            optimization_level: OptimizationLevel::Zero,
            debug_info: false,
            src_language: SourceLanguage::GLSL,
        }
    }
}

fn include_callback(
    requested: &str,
    _include_type: shaderc::IncludeType,
    _source_name: &str,
    _depth: usize,
) -> Result<ResolvedInclude, String> {
    let mut path = construct_path_and_check_exists(requested, "");
    if path.is_none() {
        unsafe {
            INCLUDE_SEM.acquire();
            if INCLUDE_DIRECTORIES.is_none() {
                INCLUDE_SEM.release();
                return Err(format!(
                    "Can't find requested path and no include directories are provided: {}",
                    requested
                ));
            }
            let inc_directories = INCLUDE_DIRECTORIES.as_ref().unwrap();
            INCLUDE_SEM.release();
            for dir in inc_directories.iter() {
                let res_path = construct_path_and_check_exists(dir, requested);
                if res_path.is_some() {
                    path = res_path;
                    break;
                }
            }
            if path.is_none() {
                return Err(format!("Can't find requested path: {}", requested));
            }
        }
    }
    let path = path.unwrap();
    let content_res = fs::read_to_string(&path);
    if content_res.is_err() {
        return Err(format!(
            "Can't read requested path {} with error {}",
            path,
            content_res.unwrap_err()
        ));
    }
    Ok(ResolvedInclude {
        resolved_name: requested.to_string(),
        content: content_res.unwrap(),
    })
}

pub fn vk_stage_to_shaderc_kind(stage: vk::ShaderStageFlags) -> Option<ShaderKind> {
    match stage {
        vk::ShaderStageFlags::VERTEX => Some(ShaderKind::Vertex),
        vk::ShaderStageFlags::FRAGMENT => Some(ShaderKind::Fragment),
        vk::ShaderStageFlags::COMPUTE => Some(ShaderKind::Compute),
        vk::ShaderStageFlags::GEOMETRY => Some(ShaderKind::Geometry),
        vk::ShaderStageFlags::TESSELLATION_CONTROL => Some(ShaderKind::TessControl),
        vk::ShaderStageFlags::TESSELLATION_EVALUATION => Some(ShaderKind::TessEvaluation),
        vk::ShaderStageFlags::RAYGEN_KHR => Some(ShaderKind::RayGeneration),
        vk::ShaderStageFlags::ANY_HIT_KHR => Some(ShaderKind::AnyHit),
        vk::ShaderStageFlags::CLOSEST_HIT_KHR => Some(ShaderKind::ClosestHit),
        vk::ShaderStageFlags::MISS_KHR => Some(ShaderKind::Miss),
        vk::ShaderStageFlags::INTERSECTION_KHR => Some(ShaderKind::Intersection),
        vk::ShaderStageFlags::CALLABLE_KHR => Some(ShaderKind::Callable),
        _ => None,
    }
}

pub fn shaderc_kind_to_vk_stage(kind: ShaderKind) -> Option<vk::ShaderStageFlags> {
    match kind {
        ShaderKind::Vertex => Some(vk::ShaderStageFlags::VERTEX),
        ShaderKind::Fragment => Some(vk::ShaderStageFlags::FRAGMENT),
        ShaderKind::Compute => Some(vk::ShaderStageFlags::COMPUTE),
        ShaderKind::Geometry => Some(vk::ShaderStageFlags::GEOMETRY),
        ShaderKind::TessControl => Some(vk::ShaderStageFlags::TESSELLATION_CONTROL),
        ShaderKind::TessEvaluation => Some(vk::ShaderStageFlags::TESSELLATION_EVALUATION),

        ShaderKind::RayGeneration => Some(vk::ShaderStageFlags::RAYGEN_KHR),
        ShaderKind::AnyHit => Some(vk::ShaderStageFlags::ANY_HIT_KHR),
        ShaderKind::ClosestHit => Some(vk::ShaderStageFlags::CLOSEST_HIT_KHR),
        ShaderKind::Miss => Some(vk::ShaderStageFlags::MISS_KHR),
        ShaderKind::Intersection => Some(vk::ShaderStageFlags::INTERSECTION_KHR),
        ShaderKind::Callable => Some(vk::ShaderStageFlags::CALLABLE_KHR),

        _ => None,
    }
}

fn construct_path_and_check_exists(path1: &str, path2: &str) -> Option<String> {
    let sum = path1.to_string() + path2;
    let mut path: Option<String> = None;
    let res = Path::new(sum.as_str()).canonicalize();
    if res.is_ok() {
        let res = res.unwrap();
        if res.exists() {
            path = Some(res.to_str().unwrap().to_string());
        }
    }
    path
}
