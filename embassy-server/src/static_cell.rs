use core::cell::UnsafeCell;
use core::mem::MaybeUninit;

pub struct StaticCell<T> {
    val: UnsafeCell<MaybeUninit<T>>,
}

unsafe impl<T> Send for StaticCell<T> {}
unsafe impl<T> Sync for StaticCell<T> {}

impl<T> StaticCell<T> {

    #[inline]
    pub const fn new() -> Self {
        Self {
            val: UnsafeCell::new(MaybeUninit::uninit()),
        }
    }

    #[inline]
    pub fn init(&'static self, val: T) -> &'static mut T {
        let maybe_uninit = unsafe { &mut *self.val.get() };
        maybe_uninit.write(val)
    }
}
