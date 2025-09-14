use ash::khr::surface;
use ash::{vk, Entry, Instance};
use sdl3::sys::vulkan::{SDL_Vulkan_CreateSurface, SDL_Vulkan_GetInstanceExtensions};
use sdl3::{
    video::{Window as SdlWindow, WindowBuilder},
    EventPump, Sdl, VideoSubsystem,
};
use std::error::Error;
use std::ffi::{c_char, CStr, CString};
use std::ptr::null;
use std::sync::{Arc, Mutex};
use sdl3::sys::everything::SDL_GetError;

// Type alias for resize callback
type ResizeCallback = Box<dyn FnMut(u32, u32) + Send + Sync>;
// Type alias for custom event handler
type EventHandler = Box<dyn FnMut(&sdl3::event::Event, &mut bool) + Send + Sync>;

pub struct Window {
    sdl: Sdl,
    sdl_video: VideoSubsystem,
    window: SdlWindow,
    surface_loader: Option<surface::Instance>,
    event_pump: EventPump,
    surface: Option<vk::SurfaceKHR>,
    resize_callback: Arc<Mutex<Option<ResizeCallback>>>,
    event_handler: Arc<Mutex<Option<EventHandler>>>,
    need_close: bool,
}

impl Window {
    pub fn get_required_instance_extensions() -> Result<Vec<CString>, Box<dyn Error>> {
        unsafe {
            // Get the number of extensions
            let mut count: u32 = 0;
            let extension_ptrs = SDL_Vulkan_GetInstanceExtensions(&mut count);
            if extension_ptrs.is_null() {
                return Err("Failed to get SDL Vulkan extensions: null pointer returned".into());
            }

            // Convert raw pointer to slice
            let extension_slice = std::slice::from_raw_parts(extension_ptrs, count as usize);

            // Convert raw C strings to CStr
            let cstr_extensions: Vec<CString> = extension_slice
                .iter()
                .map(|&ptr| CString::from(CStr::from_ptr(ptr)))
                .collect();

            Ok(cstr_extensions)
        }
    }

    pub fn new(
        width: u32,
        height: u32,
    ) -> Result<Self, Box<dyn Error>> {
        // Initialize SDL with video subsystem
        let sdl = sdl3::init()?;
        let sdl_video = sdl.video()?;

        // Create window with Vulkan and resizable flags
        let window = WindowBuilder::new(&sdl_video, "Vulkan Window", width, height)
            .position_centered()
            .resizable()
            .vulkan()
            .build()?;

        // Create event pump
        let event_pump = sdl.event_pump()?;

        // Create Vulkan surface

        Ok(Window {
            sdl,
            sdl_video,
            surface_loader: None,
            window,
            event_pump,
            surface: None,
            resize_callback: Arc::new(Mutex::new(None)),
            event_handler: Arc::new(Mutex::new(None)),
            need_close: false,
        })
    }



    pub fn get_physical_size(&self) -> (u32, u32) {
        let (width, height) = self.window.size();
        (width, height)
    }

    pub fn surface(&mut self, surface_loader: surface::Instance, instance: &Instance) -> Result<vk::SurfaceKHR, Box<dyn Error>> {
        if self.surface.is_none() {
            let surface = unsafe {
                let mut surface_handle: vk::SurfaceKHR = vk::SurfaceKHR::null();
                let result = SDL_Vulkan_CreateSurface(
                    self.window.raw(),
                    instance.handle(),
                    null(),
                    &mut surface_handle as *mut vk::SurfaceKHR,
                );
                if !result {
                    return Err(format!("Failed to create Vulkan surface {}", CStr::from_ptr(SDL_GetError()).to_str().unwrap()).into());
                }
                surface_handle
            };
            self.surface = Some(surface);
            self.surface_loader = Some(surface_loader);
        }
       Ok(self.surface.as_ref().unwrap().clone())
    }

    // Set resize callback
    pub fn set_resize_callback<F>(&mut self, callback: F)
    where
        F: FnMut(u32, u32) + Send + Sync + 'static,
    {
        let mut callback_guard = self.resize_callback.lock().unwrap();
        *callback_guard = Some(Box::new(callback));
    }

    // Set custom event handler
    pub fn set_event_handler<F>(&mut self, handler: F)
    where
        F: FnMut(&sdl3::event::Event, &mut bool) + Send + Sync + 'static,
    {
        let mut handler_guard = self.event_handler.lock().unwrap();
        *handler_guard = Some(Box::new(handler));
    }

    // Run the event loop
    pub fn poll_events(&mut self) -> Result<(), Box<dyn Error>> {
        let resize_callback = self.resize_callback.clone();
        let event_handler = self.event_handler.clone();

        for event in self.event_pump.poll_iter() {
            match event {
                sdl3::event::Event::Window { win_event, .. } => match win_event {
                    sdl3::event::WindowEvent::Resized(width, height) => {
                        if let Ok(mut callback) = resize_callback.lock() {
                            if let Some(callback_fn) = callback.as_mut() {
                                callback_fn(width as u32, height as u32);
                            }
                        }
                    }
                    sdl3::event::WindowEvent::CloseRequested => {
                        self.need_close = true;
                    }
                    _ => {}
                },
                sdl3::event::Event::Quit { .. } => {
                    self.need_close = true;
                }
                _ => {}
            }

            // Call custom event handler if set
            if let Ok(mut handler) = event_handler.lock() {
                if let Some(handler_fn) = handler.as_mut() {
                    handler_fn(&event, &mut self.need_close);
                }
            }
        }

        Ok(())
    }

    pub fn need_close(&self) -> bool {
        self.need_close
    }
}

impl Drop for Window {
    fn drop(&mut self) {
        unsafe {
            if self.surface.is_some() {
                self.surface_loader.as_ref().unwrap().destroy_surface(self.surface.unwrap(), None);
            }
        }
    }
}
