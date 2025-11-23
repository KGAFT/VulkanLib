use crate::util::vl_semaphore::VlSemaphore;
use shaderc::{
    CompileOptions, Compiler, OptimizationLevel, ResolvedInclude, ShaderKind, SourceLanguage,
    SpirvVersion,
};
use std::fs;
use std::path::{Path, PathBuf};
use std::sync::{Arc, Mutex};

static mut INCLUDE_DIRECTORIES: Option<Vec<String>> = None;
static mut INCLUDE_SEM: VlSemaphore = VlSemaphore::new(1);
pub struct ShaderLoader {
    compiler: Compiler,
    optimization_level: OptimizationLevel,
    debug_info: bool,
    src_language: SourceLanguage,
}

impl ShaderLoader {
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
            optimization_level: OptimizationLevel::Zero,
            debug_info: false,
            src_language: SourceLanguage::GLSL,
        }
    }

    pub fn set_optimization_level(&mut self, level: OptimizationLevel) {
        self.optimization_level = level;
    }

    pub fn set_source_language(&mut self, src_language: SourceLanguage) {
        self.src_language = src_language;
    }

    pub fn set_debug_info(&mut self, debug: bool) {
        self.debug_info = debug;
    }

    pub fn read_and_compile_shader(&self, path: &Path, kind: ShaderKind, entry_point_name: &str) -> Option<Vec<u32>> {
        let content = fs::read_to_string(path);
        if content.is_err() {
            return None;
        }
        let content = content.unwrap();
        Some(self.compile_shader(content.as_str(), kind, path.file_name().unwrap().to_str()?, entry_point_name).unwrap_or(Vec::new()))
    }
    pub fn compile_shader(
        &self,
        source: &str,
        kind: ShaderKind,
        input_file_name: &str,
        entry_point_name: &str,
    ) -> shaderc::Result<Vec<u32>> {
        let compile_options = self.get_compile_options();
        let res = self.compiler.compile_into_spirv(
            source,
            kind,
            input_file_name,
            entry_point_name,
            Some(&compile_options),
        )?;
        Ok(Vec::from(res.as_binary()))
    }

    fn get_compile_options(&self) -> CompileOptions {
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

fn include_callback(
    requested: &str,
    include_type: shaderc::IncludeType,
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
                let mut res_path = construct_path_and_check_exists(dir, requested);
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
