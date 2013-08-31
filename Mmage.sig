signature Mmage =
sig
   type colour = {red: int, green: int, blue: int}
   type color = colour

   type pixel

   val getColour : pixel -> colour
   val getRed: pixel -> int
   val getGreen: pixel -> int
   val getBlue: pixel -> int

   type image

   val make : int * int * colour -> image
   val readFromFile : string -> image
   val writeToFile : image -> string -> unit

   val width : image -> int
   val height: image -> int

   type index = int * int
   type rect = index * index

   val transform      : image * (pixel -> colour) -> image
   val transformi     : image * (pixel * index -> colour) -> image
   val transformRect  : image * rect * (pixel -> colour) -> image
   val transformRecti : image * rect * (pixel * index -> colour) -> image

   val blit : image * rect * image * index -> image

   val setPixel : image * index * colour -> image

   val show : image -> unit
end
