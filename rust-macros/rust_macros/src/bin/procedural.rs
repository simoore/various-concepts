use proc_macros::HelloMacro;
use proc_macros::trace;
use proc_macros::print_and_replace;

#[derive(HelloMacro)]
struct Pancakes;

trait HelloTrait {
    fn hello_macro();
}

#[allow(dead_code)]
#[trace(some_arg)] // This attribute macro prints code at compile time.
fn baz() {}

fn main() {
    // The implementation of the hello macro trait is performed by the hello macro.
    Pancakes::hello_macro();

    // At complile time prints "Inputs: 100" and generates a definition of the add(..) function.
    print_and_replace!(100); 

    // Not an error as the macro call above brings 'add' into scope.
    println!("The result {}", add(1,2)); 
}
