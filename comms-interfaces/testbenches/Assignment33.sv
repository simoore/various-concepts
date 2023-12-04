`timescale 1ns / 1ps

module Assignment33;

// This is a fixed sized array
int unsigned arr1[15] = '{default : 2};

// This is a dynamically sized array
int arr2[];

int unsigned queue[$];

initial begin

    foreach (arr1[i]) begin
        arr1[i] = $urandom();
    end
    
    $display("arr1 is : %0p", arr1);
    
    // This creates a dynamic array
    arr2 = new[7];
    
    foreach (arr2[i]) begin
        arr2[i] = 7*(i + 1);
    end
    
    #20;
    
    // This recreates a dynamic array and copies the elements of another array into it
    arr2 = new[20](arr2);
    
    for (int i = 7; i < 20; i++) begin
        arr2[i] = 5 * (i - 6);
    end
    
    $display("arr2 is : %0p", arr2);
    
    // We are going to push the elements of arr1 to the queue
    foreach (arr1[i]) begin
        queue.push_front(arr1[i]);
    end
    
    $display("queue is : %0p", queue);
    
end    
    
endmodule
