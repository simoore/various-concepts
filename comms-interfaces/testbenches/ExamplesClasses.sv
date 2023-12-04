class Second;
    int data;
    
    function nonVirtualDisplay();
        $display("From Base Class");
    endfunction
    
    virtual function virtualDisplay();
        $display("From Base Class");
    endfunction
endclass

class First;

    bit [2:0] data;
    bit [1:0] data2;
    int data3;
    
    // Member fields, if you want to initialize a class member
    // call the members constructor in the constructor
    local Second second;
    
    // Custom constructor
    function new(input int data3 = 32);
        this.data3 = data3;
        second = new();
    endfunction
    
    task display();
        $display("Value of data : %0d and data2 : %0d", data, data2);
        $display("Value of data3 : %0d", data3);
    endtask

endclass

// This is how you implement inheritance. SystemVerilog has virtual functions.
class Derived extends Second;
    function nonVirtualDisplay();
        $display("From Derived Class");
    endfunction
    
    function virtualDisplay();
        $display("From Derived Class");
    endfunction
endclass

module ExamplesClasses;

    function bit [4:0] add(input bit [3:0] a, b);
        return a + b;
    endfunction
    
    // Pass arrays by reference to avoid copying the entire array
    // When passing by reference, you need to specify that the function use automatic allocation
    function automatic void initArr(ref bit [3:0] a[16]);
        for (int i = 0; i < 16; i++) begin
            a[i] = i;
        end
    endfunction
    
    First first, first2;
    bit [4:0] res = 0;
    bit [3:0] arr[16];
    
    Second base;
    Derived derived;
    
    initial begin
        
        // Need to construct the class.
        first = new();
        #1;
        first.display();
        
        res = add(4'b0100, 4'b0010);
        $display("Value of addition : %0d", res);
        
        initArr(arr);
        for (int i = 0; i < 16; i++) begin
            $display("arr[%0d] : %0d", i, arr[i]);
        end
        
        // This performs a shallow copy as the handler of second in the class points to the
        // same object. For a deep copy, you need to create a custom copy method to create
        // new instances of the member classes.
        first2 = new first;
        
        // The difference between virtual and non-virtual inheritance.
        derived = new();
        base = derived;
        base.nonVirtualDisplay();   // This calls the base class display because it is using its compile time type
        base.virtualDisplay();      // This calls the derived class display because it is using it runtime type
        
    end

endmodule
