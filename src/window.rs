use ash::vk;
use ash::Instance;
use ash::khr::surface;

use glfw::{Action, GlfwReceiver, Key, WindowEvent};
use std::error::Error;
use std::ffi::CString;
use std::ptr::null;
use std::sync::{Arc, Mutex};
use ash::vk::Handle;
use glfw::ffi::VkResult_VK_SUCCESS;

type ResizeCallback = Box<dyn FnMut(u32, u32) + Send + Sync>;
type EventHandler = Box<dyn FnMut(&glfw::WindowEvent, &mut bool) + Send + Sync>;

pub struct Window {
    glfw: glfw::Glfw,
    window: glfw::PWindow,
    events: GlfwReceiver<(f64, glfw::WindowEvent)>,

    surface_loader: Option<surface::Instance>,
    surface: Option<vk::SurfaceKHR>,

    resize_callback: Arc<Mutex<Option<ResizeCallback>>>,
    event_handler: Arc<Mutex<Option<EventHandler>>>,

    need_close: bool,
}

impl Window {
    pub fn get_required_instance_extensions() -> Result<Vec<CString>, Box<dyn Error>> {
        let glfw = glfw::init_no_callbacks()?;

        let ext_list = glfw.get_required_instance_extensions()
            .ok_or("GLFW could not provide Vulkan extensions")?;

        Ok(ext_list
            .iter()
            .map(|s| CString::new(s.as_str()).unwrap())
            .collect())
    }

    pub fn new(width: u32, height: u32) -> Result<Self, Box<dyn Error>> {
        let mut glfw = glfw::init_no_callbacks()?;

        glfw.window_hint(glfw::WindowHint::ClientApi(glfw::ClientApiHint::NoApi));
        glfw.window_hint(glfw::WindowHint::Resizable(true));
        glfw.window_hint(glfw::WindowHint::Decorated(true));
        glfw.window_hint(glfw::WindowHint::TransparentFramebuffer(true));
        let (mut window, events) = glfw
            .create_window(width, height, "Vulkan Window", glfw::WindowMode::Windowed)
            .ok_or("Failed to create window")?;

        window.set_all_polling(true);
        window.show();
        Ok(Window {
            glfw,
            window,
            events,

            surface_loader: None,
            surface: None,

            resize_callback: Arc::new(Mutex::new(None)),
            event_handler: Arc::new(Mutex::new(None)),

            need_close: false,
        })
    }

    pub unsafe fn create_surface(
        &mut self,
        surface_loader: surface::Instance,
        instance: &Instance,
    ) -> Result<vk::SurfaceKHR, Box<dyn Error>> {
        if self.surface.is_some() {
            return Ok(self.surface.unwrap());
        }
        let mut surface: glfw::ffi::VkSurfaceKHR = 0 as glfw::ffi::VkSurfaceKHR ;

        let raw_surface = self.window.create_window_surface(instance.handle().as_raw() as _, null(), &mut surface);
        if raw_surface!=VkResult_VK_SUCCESS{
            return Err(format!("Failed to create surface {}", raw_surface).into());
        }

        let surface = vk::SurfaceKHR::from_raw(surface as u64);

        self.surface = Some(surface);
        self.surface_loader = Some(surface_loader);

        Ok(surface)
    }

    pub fn get_physical_size(&self) -> (u32, u32) {
        let (w, h) = self.window.get_size();
        (w as u32, h as u32)
    }

    pub fn set_resize_callback<F>(&mut self, callback: F)
    where
        F: FnMut(u32, u32) + Send + Sync + 'static,
    {
        *self.resize_callback.lock().unwrap() = Some(Box::new(callback));
    }

    pub fn set_event_handler<F>(&mut self, handler: F)
    where
        F: FnMut(&glfw::WindowEvent, &mut bool) + Send + Sync + 'static,
    {
        *self.event_handler.lock().unwrap() = Some(Box::new(handler));
    }

    pub fn poll_events(&mut self) -> Result<(), Box<dyn Error>> {
        self.glfw.poll_events();

        let resize_cb = self.resize_callback.clone();
        let event_handler = self.event_handler.clone();

        for (_, event) in glfw::flush_messages(&self.events) {
            match &event {
                WindowEvent::Size(w, h) => {
                    if let Some(cb) = resize_cb.lock().unwrap().as_mut() {
                        cb(*w as u32, *h as u32);
                    }
                }

                WindowEvent::Close => {
                    self.need_close = true;
                }

                WindowEvent::Key(Key::Escape, _, Action::Press, _) => {
                    self.need_close = true;
                }

                _ => {}
            }

            if let Some(handler) = event_handler.lock().unwrap().as_mut() {
                handler(&event, &mut self.need_close);
            }
        }

        Ok(())
    }

    pub fn clear_resize_callbacks(&mut self){
        *self.resize_callback.lock().unwrap() = None;
    }

    pub fn need_close(&self) -> bool {
        self.need_close
    }
}

impl Drop for Window {
    fn drop(&mut self) {
        unsafe {
            if let Some(surface) = self.surface {
                if let Some(loader) = &self.surface_loader {
                    loader.destroy_surface(surface, None);
                }
            }
        }
    }
}
