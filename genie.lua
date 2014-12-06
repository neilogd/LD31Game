
-- Run psybrus scripts.
psybrusSDK = "Psybrus"
dofile( "Psybrus/Scripts/Psybrus.lua" )

-- Solution.
PsySolutionGame( "LD31Game" )

-- Build externals.
dofile ("Psybrus/External/genie.lua")

-- Build engine.
dofile ("Psybrus/Engine/genie.lua")

-- Build game source.
dofile ("./Source/genie.lua")

