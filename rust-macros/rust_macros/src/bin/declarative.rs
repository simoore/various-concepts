macro_rules! greetings {
    (struct $x:ident {}) => {
        println!("Hello from struct {}", stringify!($x));
    };
    ($x:expr) => {
        println!("Hello, {}", $x);
    };
}

macro_rules! sum {
    ($a:expr) => {
        $a
    };
    ($a:expr, $b:expr) => {
        $a + $b
    };
    ($a:expr, $($b:tt)*) => {
        $a + sum!($($b)*)
    }
}

fn main() {
    greetings!("World");
    greetings!(
        struct G {}
    );
    println!("Sum is {}", sum!(1, 2, 3, 4, 5))
}
