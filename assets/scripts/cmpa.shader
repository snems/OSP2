hud/lowhp
{
	nopicmip
	{
		map gfx/2d/grad_right.tga
		rgbGen const ( 1 0 0 )
		blendFunc blend
		alphaGen wave sin 0.85 0.15 0 7
	}
}

gfx/2d/grad_left
{
	nopicmip
	{
		clampmap gfx/2d/grad_left.tga
		blendfunc blend
		rgbgen vertex
	}
}

gfx/2d/grad_right
{
	nopicmip
	{
		clampmap gfx/2d/grad_right.tga
		blendfunc blend
		rgbgen vertex
	}
}

