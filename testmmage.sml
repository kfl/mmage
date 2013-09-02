fun showprofile () =
    let val profile = Mmage.readFromFile "profile.jpg"
    in  Mmage.show profile
    end

fun redden pixel = let val {red,green,blue} = Mmage.getColour pixel
                   in  {red=255,green=green,blue=blue}
                   end

fun reddenprofile () =
    let val profile = Mmage.readFromFile "profile.jpg"
    in  Mmage.show profile
      ; Mmage.show (Mmage.transform(profile, redden))
    end

fun whiteboxtest () =
    let val whitebox = Mmage.make(500, 500, {red=250,green=250,blue=250})
    in  app Mmage.show [ whitebox
                       , Mmage.transform(whitebox, redden)
                       ]
    end

fun setpixels () =
    let val profile = Mmage.readFromFile "profile.jpg"
        val green = {red=0, green=255, blue=0}
        fun setpix m n img =
            let fun loopx 0 img = img
                  | loopx i img =
                    let fun loopy 0 img = img
                          | loopy j img = loopy (j-1) (Mmage.setPixel(img, (i, j), green))
                    in  loopx (i-1) (loopy n img)
                    end
            in  loopx m img end
    in  Mmage.show (setpix 50 50 profile)
    end

val _ = setpixels()
