ifdef __FLAT__
        .386p
        .model  small, pascal
elseifdef MSMALL
        .model  small, pascal
elseifdef MMEDIUM
        .model  medium, pascal
elseifdef MLARGE
        .model  large, pascal
elseifdef MCOMPACT
        .model  compact, pascal
endif

