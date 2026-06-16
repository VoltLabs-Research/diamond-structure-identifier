# Identify Diamond Structure

Identifies cubic and hexagonal diamond structures using 2nd-nearest-neighbour shell CNA, labelling core atoms and their first/second neighbour shells.

## Install

```bash
vpm install @voltlabs/identify-diamond
```

## CLI

```bash
identify-diamond <input_dump> [output_base]
```

| Argument | Required | Default | Description |
|---|---|---|---|
| `<input_dump>` | yes | — | Input LAMMPS dump. |
| `[output_base]` | no | derived from input | Base path for output files. |

## Exports

| Output file | Exposure | Exporter → artifact |
|---|---|---|
| `{output_base}_atoms.parquet` | Diamond Structure | AtomisticExporter → glb |
| `{output_base}_identify_diamond.parquet` | Diamond Summary | — (listing-only) |

---

Full input contract and examples: https://docs.voltcloud.dev/docs/plugins
