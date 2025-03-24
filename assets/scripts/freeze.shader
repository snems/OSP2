freezeShader
{
	nopicmip
	deformVertexes wave 100 sin 2 0 0 0
	{
		map gfx/misc/freeze.jpg
		blendfunc add
		rgbGen const ( 0.95 1 1 )
		tcMod scroll 0.1 0.1
		tcGen environment
	}
}

freezeMarkShader
{
	nopicmip
	polygonoffset
	{
		clampmap gfx/damage/freeze_stain.tga
		blendfunc gl_src_alpha gl_one_minus_src_alpha
		rgbgen identitylighting
		alphagen vertex
	}
}

spawnShader
{
        deformVertexes wave 100 sin 3 0 0 0
        {
                map menu/art/fx_grn.tga
                blendfunc GL_ONE GL_ONE
                tcGen environment
                tcmod rotate 30
                //tcMod turb 0 0.2 0 .2
                tcmod scroll 1 .1
        }
}

iceTrail
{
        
        nopicmip                        // make sure a border remains
        entityMergable          // allow all the sprites to be merged together
        {
                clampmap gfx/damage/blood_spurt.tga
                blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
                rgbGen          vertex
                alphaGen        vertex
        }
}
