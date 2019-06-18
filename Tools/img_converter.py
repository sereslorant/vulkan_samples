
import png;

import array;

FORMAT_RGBAF32 = 0;

def convert(filename):
    with open(filename + ".png","rb") as f:
        r=png.Reader(file=f);
        #print(r.asRGB8());
        Width,Height,Pixels,Meta = r.asFloat();
        
        ##print("Width:  " + str(Width));
        ##print("Height: " + str(Height));
        ##print("Pixels: " + str(Pixels));
        
        with open(filename,"wb") as f:
            Header  = array.array("I",[FORMAT_RGBAF32,Width,Height]);
            
            Header.tofile(f);
            
            Payload = array.array("f")
            i = 0;
            for Row in Pixels:
                for Pixel in Row:
                    Payload.append(Pixel);
                    i += 1;
                    if Meta["planes"] == 3 and i == 3:
                        Payload.append(1.0);
                        i = 0;
            
            Payload.tofile(f);
            
            ##print(Header);
            ##print(Payload);

#convert("FileTest0");
#convert("FileTest1");
#convert("FileTest2");
#convert("FileTest3");

convert("Img0");
convert("Img1");
convert("Img2");
convert("Img3");

convert("Img0Grn");
convert("Img1Grn");
convert("Img2Grn");
convert("Img3Grn");

convert("Img0Blu");
convert("Img1Blu");
convert("Img2Blu");
convert("Img3Blu");

convert("Img0Brn");
convert("Img1Brn");
convert("Img2Brn");
convert("Img3Brn");
