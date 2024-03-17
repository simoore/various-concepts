#[cfg(test)]
pub static CODE: &str = "\
sim
   if (energy > 0) then
      if (awakeDaily > 16) then
         rest 8
      else 
         if (energy > 100) then
            if (rand < 125) then
               breed N
            else 
               if (rand < 143) then
		  breed NE
	       else 
                  if (rand < 167) then
		     breed E
 		  else
		     if (rand < 200) then
		        breed SE
		     else
			if (rand < 250) then
			   breed S
			else
			   if (rand < 333) then
               		      breed SW
                           else 
               		      if (rand < 500) then
		  	         breed W
	       		      else 
		     		 breed NW
			      end
 		  	   end
         	        end
            	     end
                  end
               end
            end
         else
            if (rand < 125) then
               hunt N
            else 
               if (rand < 143) then
		  hunt NE
	       else 
                  if (rand < 167) then
		     hunt E
 		  else
		     if (rand < 200) then
		        hunt SE
		     else
			if (rand < 250) then
			   hunt S
			else
			   if (rand < 333) then
               		      hunt SW
                           else 
               		      if (rand < 500) then
		  	         hunt W
	       		      else 
		     		 hunt NW
			      end
 		  	   end
         	        end
            	     end
                  end
               end
            end
         end
      end
   end
end
    ";
