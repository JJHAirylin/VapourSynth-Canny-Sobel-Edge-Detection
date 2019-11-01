import vapoursynth as vs
core = vs.get_core()

clip = core.ffms2.Source(source='input.m2ts')
clip = core.std.Transpose(clip)

clip.set_output()
