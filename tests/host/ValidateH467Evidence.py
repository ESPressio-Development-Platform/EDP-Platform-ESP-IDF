#!/usr/bin/env python3
"""Frozen H467 source/quality/native evidence; no device or current-source action."""
from pathlib import Path
import json,hashlib
root=Path(__file__).resolve().parents[2];e=root/'docs/evidence/H467';load=lambda n:json.loads((e/n).read_text())
for rel,sha in load('FINAL-SOURCE-SHA256.json').items():
 assert hashlib.sha256((e/('final-'+rel.replace('/','-')+'.txt')).read_bytes()).hexdigest()==sha,rel
p=load('VALIDATION-PROVENANCE.json');assert not p['hardware'] and not p['root']
for rid in p['runs']:assert load(rid+'-state.json')['state']=='PASS' and load(rid+'-state.json')['exit_code']=='0'
for name,count in [('H467-RELEASE-RESULTS.json',24),('H467-SANITIZE-RESULTS.json',24),('H467-ANALYZE-RESULTS.json',12)]:
 rows=load(name);assert len(rows)==count and all(not row['hardware'] for row in rows)
for name,targets in [('H467-DEMO-SOURCE-BASIS.json',4),('H467-NATIVE-SOURCE-BASIS.json',6)]:
 d=load(name);assert d['equal'] and d['before']==d['after'] and d['targets']==targets and not d['hardware']
g=load('DEPENDENCY-REVIEW.json');assert len(g['owning_graph'])==8 and len(g['complete_native_application_graph'])==23 and g['acyclic'] and g['new_mandatory_edges']==[['EDP-Platform-ESP-IDF','EDP-Clock']]
assert load('UNCHANGED-EXISTING-NATIVE-PROVIDERS.json')['all_existing_provider_files_unchanged']
placement=load('H467-CALLBACK-PLACEMENT.json');assert len(placement)==2
assert all(v['gate_exit']==0 and 'IRQ_CLOCK_ELF_PLACEMENT_PASS' in v['stdout'] for v in placement.values())
assert placement['esp32s3']['missing_and_changed_sdk_binding_refused']
print('H467_FROZEN_NATIVE_TIMEBASE_SAME_COUNTER_MODEL_QUALITY_TEN_NATIVE_COMPILES_STRICT_CALLBACK_GRAPH_PASS|hardware=0|physical_rate_measured=0|root=0')
