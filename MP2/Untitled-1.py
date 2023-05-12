
x = [( 0x00, 0x00, 0x00 ),( 0x00, 0x00, 0x2A ),   
    ( 0x00, 0x2A, 0x00 ),( 0x00, 0x2A, 0x2A ),   
    ( 0x2A, 0x00, 0x00 ),( 0x2A, 0x00, 0x2A ),
    ( 0x2A, 0x15, 0x00 ),( 0x2A, 0x2A, 0x2A ),
    ( 0x15, 0x15, 0x15 ),( 0x15, 0x15, 0x3F ),
    ( 0x15, 0x3F, 0x15 ),( 0x15, 0x3F, 0x3F ),
    ( 0x3F, 0x15, 0x15 ),( 0x3F, 0x15, 0x3F ),
    ( 0x3F, 0x3F, 0x15 ),( 0x3F, 0x3F, 0x3F ),
    ( 0x00, 0x00, 0x00 ),( 0x05, 0x05, 0x05 ),   
    ( 0x08, 0x08, 0x08 ),( 0x0B, 0x0B, 0x0B ),   
    ( 0x0E, 0x0E, 0x0E ),( 0x11, 0x11, 0x11 ),
    ( 0x14, 0x14, 0x14 ),( 0x18, 0x18, 0x18 ),
    ( 0x1C, 0x1C, 0x1C ),( 0x20, 0x20, 0x20 ),
    ( 0x24, 0x24, 0x24 ),( 0x28, 0x28, 0x28 ),
    ( 0x2D, 0x2D, 0x2D ),( 0x32, 0x32, 0x32 ),
    ( 0x38, 0x38, 0x38 ),( 0x3F, 0x3F, 0x3F ),
    ( 0x3F, 0x3F, 0x3F ),( 0x3F, 0x3F, 0x3F ),   
    ( 0x00, 0x00, 0x3F ),( 0x00, 0x00, 0x00 ),   
    ( 0x00, 0x00, 0x00 ),( 0x00, 0x00, 0x00 ),
    ( 0x00, 0x00, 0x00 ),( 0x00, 0x00, 0x00 ),
    ( 0x00, 0x00, 0x00 ),( 0x00, 0x00, 0x00 ),
    ( 0x00, 0x00, 0x00 ),( 0x00, 0x00, 0x00 ),
    ( 0x00, 0x00, 0x00 ),( 0x00, 0x00, 0x00 ),
    ( 0x00, 0x00, 0x00 ),( 0x00, 0x00, 0x00 ),
    ( 0x10, 0x08, 0x00 ),( 0x18, 0x0C, 0x00 ),   
    ( 0x20, 0x10, 0x00 ),( 0x28, 0x14, 0x00 ),   
    ( 0x30, 0x18, 0x00 ),( 0x38, 0x1C, 0x00 ),
    ( 0x3F, 0x20, 0x00 ),( 0x3F, 0x20, 0x10 ),
    ( 0x20, 0x18, 0x10 ),( 0x28, 0x1C, 0x10 ),
    ( 0x3F, 0x20, 0x10 ),( 0x38, 0x24, 0x10 ),
    ( 0x3F, 0x28, 0x10 ),( 0x3F, 0x2C, 0x10 ),
    ( 0x3F, 0x30, 0x10 ),( 0x3F, 0x20, 0x10 )]

for tup in x:
    r, g, b = tup
    newR = (int)((r+0x3F) /2)
    newG = (int)((g+0x3F) /2)
    newB = (int)((b+0x3F) /2)
    print("{" + hex(newR) + "," + hex(newG) + "," + hex(newB) + "}")


        
    