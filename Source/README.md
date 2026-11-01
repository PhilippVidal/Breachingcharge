# Breachingcharge source baseline

`HDSN_BreachingCharge` contains the files recovered from the original distributed
`Breachingcharge/Addons/HDSN_BreachingCharge.pbo` at upstream commit
`895929e4fa355b6c70d815569153480a669534f2`.

PBO SHA-256: `eabc5c4a7421bcfdb544f4c86de0a8cc627951320dba1e7d7dd5d3faed250ae7`.

This is the source tree for the existing original mod, with its existing prefix
`HDSN_BreachingCharge`. Scripts, language files and assets retain their original
paths. It is not a separately loaded compatibility add-on. No additional
CfgPatches or CfgMods class is introduced.

Text is normalized to UTF-8 without BOM, LF and no trailing whitespace. The
generated DeRap header is omitted from config.cpp. The prefix file contains the
original prefix. Binary assets are unchanged. Models remain in their distributed
format; this import does not recover their original authoring projects.

Build the existing mod from this directory with the original prefix and bundled
assets, using a separate staging/output directory. Existing distributed models
are already binarized. Preserve the repository's original PBO and signature as
the release baseline until a maintainer builds and signs a new release. The old
signature does not authenticate a rebuilt PBO. Upstream usage restrictions remain
unchanged.

The common source-import commit is separated from each functional fix so that
the actual code change can be reviewed independently. It is shared by all five
fix branches and only needs to be merged once.
