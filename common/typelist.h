DEF_TP(UNDEF    , 1, 1, "<UNDEF>"    , "<UNDEF>"    , 0                              )
DEF_TP(VOID     , 0, 0, "void"       , "void"       , 0                              )

DEF_TP(BOOL     , 1, 1, "boolean"    , "i1"         ,         VTF_SCL|VTF_INT|VTF_UNS)

DEF_TP(INT      , 4, 4, "int"        , "i32"        , VTF_ARI|VTF_SCL|VTF_INT        )

DEF_TP(FLOAT    , 4, 4, "float"      , "float"      , VTF_ARI        |VTF_FLT        )
DEF_TP(DOUBLE   , 8, 8, "double"     , "double"     , VTF_ARI        |VTF_FLT        )

DEF_TP(ARRAY    , 4, 4, "array"      , "array"      ,         VTF_SCL        |VTF_UNS)
DEF_TP(FNC      , 0, 0, "function"   , "function"   , 0                              )
DEF_TP(PTR      , 4, 4, "pointer"    , "pointer"    ,         VTF_SCL        |VTF_UNS)
