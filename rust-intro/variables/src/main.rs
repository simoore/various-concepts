fn main() {
    //some_basic_examples();
    //breaking_loop_example();
    strings();
}

fn strings() {
    let mut s = String::from("hello");
    s.push_str(", world!"); // push_str() appends a literal to a String
    println!("{}", s); // This will print `hello, world!`

    // Rust uses move semantics by default. Use the clone() function to do a deep copy.
    let s1 = String::from("hello");
    let _s2 = s1;

    // Compile error because s1 has been invalidated by the move to s2.
    //println!("{}, world!", s1);

    let s1 = String::from("hello");
    let s2 = s1.clone();
    println!("s1 = {}, s2 = {}", s1, s2);

    let len = calculate_length(&s1);
    println!("The length of '{}' is {}.", s1, len);

    let test_string = String::from("This is a test.");
    let first = first_word(&test_string);
    println!("The first word is: {}", first)
}

// A reference allows you to pass a value with out taking ownership of it.
// References are by default immutable. Use the mut keyword to make it mutable.
// A variable can only have one active mutable reference to it at any one time.
fn calculate_length(s: &String) -> usize {
    s.len()
}

fn first_word(s: &str) -> &str {
    let bytes = s.as_bytes();

    for (i, &item) in bytes.iter().enumerate() {
        if item == b' ' {
            return &s[0..i];
        }
    }

    &s[..]
}

#[allow(dead_code)]
fn some_basic_examples() {
    // x is immutable by default, use the mut keyword to make it mutable
    let mut x = 5;  
    println!("The value of x is: {}", x);
    x = 6;
    println!("The value of x is: {}", x);

    // * const must have a type annotation
    // * they can be declared int the global scope
    // * value must be know at compile time
    #[allow(dead_code)]
    const THREE_HOURS_IN_SECONDS: u32 = 60 * 60 * 3; 

    // We can shadow a variable and re-assign a label at any time.
    // When we shadow a variable, it is essentially a new variable and can have a different type.
    let x = 5;
    let x = x + 1;
    {
        let x = x * 2;
        println!("The value of x in the inner scope is: {}", x);
    }
    println!("The value of x is: {}", x);

    // Rust's char type is 4 bytes in size and represents a unicode character.
    #[allow(unused)]
    let heart_eyed_cat = '😻';

    // Example of using a tuple.
    let x: (i32, f64, u8) = (500, 6.4, 1);
    let five_hundred = x.0;
    let six_point_four = x.1;
    let one = x.2;
    println!("The tuple is {} {} {}", five_hundred, six_point_four, one);

    // Element access in an array
    let a = [1, 2, 3, 4, 5];

    println!("Please enter an array index.");

    //let mut index = String::new();

    // io::stdin()
    //     .read_line(&mut index)
    //     .expect("Failed to read line");
    let index = "1";

    let index: usize = index
        .trim()
        .parse()
        .expect("Index entered was not a number");

    let element = a[index];

    println!(
        "The value of the element at index {} is: {}",
        index, element
    );
}

#[allow(dead_code)]
fn breaking_loop_example() {
    let mut count = 0;
    'counting_up: loop {
        println!("count = {}", count);
        let mut remaining = 10;

        loop {
            println!("remaining = {}", remaining);
            if remaining == 9 {
                break;
            }
            if count == 2 {
                break 'counting_up;
            }
            remaining -= 1;
        }

        count += 1;
    }
    println!("End count = {}", count);
}