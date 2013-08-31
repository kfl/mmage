structure Mmage :> Mmage =
struct

local
    structure D = Dynlib
    val mmagelibname="mmage.so"
    val path = case Process.getEnv "MMAGEHOME" of
                   SOME p => Path.concat (p, mmagelibname)
                 | NONE   => Path.concat (".", mmagelibname)

    val hdl  = D.dlopen {lib = path, flag = D.RTLD_NOW, global = false}

    val symb = D.dlsym hdl
    fun app1 name = Dynlib.app1 (symb ("mmage_"^name))
    fun app2 name = Dynlib.app2 (symb ("mmage_"^name))
    fun app3 name = Dynlib.app3 (symb ("mmage_"^name))
    fun app4 name = Dynlib.app4 (symb ("mmage_"^name))
    fun app5 name = Dynlib.app5 (symb ("mmage_"^name))

    prim_type surface

    val init : unit -> unit = app1 "init"
    val image_load : string -> surface = app1 "image_load"
    val image_savebmp : surface -> string -> unit = app2 "image_savebmp"
    val show_surface : surface -> unit = app1 "show"
    val get_width : surface -> int = app1 "get_width"
    val get_height : surface -> int = app1 "get_height"
    val get_pitch : surface -> int = app1 "get_pitch"
    val get_bytes_per_pixel : surface -> int = app1 "get_bytes_per_pixel"
    val surface_lock : surface -> unit = app1 "surface_lock"
    val surface_unlock : surface -> unit = app1 "surface_unlock"
    val get_rgb_ : surface -> int -> int * int * int = app2 "get_rgb"
    val set_rgb_ : surface -> int -> int -> int -> int -> unit = app5 "set_rgb"
    val copy_surface : surface -> surface = app1 "copy_surface"
    val create_surface : int -> int -> int -> int -> int -> surface = app2 "create_surface"

    fun check_index surface x y = x >= 0 andalso x < get_width surface
                                  andalso y >= 0 andalso x < get_height surface
                                  orelse raise Subscript

in


type colour = {red: int, green: int, blue: int}
type color = colour

val min = Int.min
val max = Int.max
fun bytecap n = min(255, max(0, n))
fun normalise {red,green,blue} = (bytecap red, bytecap green, bytecap blue)

(* An offset in bytes into the surface's pixel array *)
type offset = int
type pixel = int * surface

fun notimplemented name _ = raise Fail (name ^ " is not implemented yet")

val getColour : pixel -> colour
    = fn (idx, surface) => let val (r, g, b) = get_rgb_ surface idx
                           in {red=r, green=g, blue=b} end

val getRed: pixel -> int   = fn (idx, surface) => #1 (get_rgb_ surface idx)
val getGreen: pixel -> int = fn (idx, surface) => #2 (get_rgb_ surface idx)
val getBlue: pixel -> int  = fn (idx, surface) => #3 (get_rgb_ surface idx)

type image = surface

val make : int * int * colour -> image =
    fn(w,h, col) => let val (r,g,b) = normalise col
                    in  w > 0 andalso h > 0 orelse raise Subscript
                      ; create_surface w h r g b
                    end
val readFromFile : string -> image = image_load
val writeToFile : image -> string -> unit = image_savebmp

val width : image -> int = get_width
val height: image -> int = get_height

type index = int * int
type rect = index * index

fun withUnlocked img f = ( surface_unlock img
                         ; (f () before surface_unlock img)
                           handle ?? => (surface_unlock img; raise ??))

val transform      : image * (pixel -> colour) -> image
    = fn (img, f) =>
         let val bpp = get_bytes_per_pixel img
             val bytes = width img * height img * bpp
             val clone = copy_surface img
             fun loop i = if i < bytes then let val (r,g,b) = normalise (f (i, img))
                                            in set_rgb_ clone i r g b
                                             ; loop (i+bpp)
                                            end
                          else ()
         in  withUnlocked img (fn () =>
             withUnlocked clone (fn () =>
             loop 0))
           ; clone
         end

val transformi     : image * (pixel * index -> colour) -> image = notimplemented "transformi"
val transformRect  : image * rect * (pixel -> colour) -> image = notimplemented "transformRect"
val transformRecti : image * rect * (pixel * index -> colour) -> image = notimplemented "transformRecti"

val blit : image * rect * image * index -> image = notimplemented "blit"

val setPixel : image * index * colour -> image
    = fn (img, (x,y), col) =>
         let val (r,g,b) = normalise col
             val bpp = get_bytes_per_pixel img
             val pitch = get_pitch img
             val idx = y * pitch + x * bpp
             val clone = copy_surface img
         in  check_index img x y
           ; withUnlocked clone (fn () => set_rgb_ clone idx r g b)
           ; clone
         end

val show : image -> unit = show_surface

val _ = init()
end
end
