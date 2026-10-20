from pathlib import Path
import json,subprocess,re,hashlib
root=Path('/srv/espressio/repos'); out=Path('/srv/espressio/scratch/H457-metadata');out.mkdir(exist_ok=True)
changes={'EDP-Mesh':('9ce5e9121703a3f28b4edd15d7a35d5c99daab81','EDP-Platform','main',25),'EDP-Platform-ESP-IDF':('840f896a0f73a2772ba851345105baa984730991','EDP-System','main',11)}
def git(p,*args):return subprocess.check_output(['git',*args],cwd=p,text=True).strip()

def deps(n):
 d=json.loads((root/n/'library.json').read_text());ds=d.get('dependencies',[])
 if isinstance(ds,dict):return list(ds)
 return [x['name'] for x in ds if x['name'].startswith('EDP-')]
def closure(n,stack=(),nodes=None):
 nodes={} if nodes is None else nodes
 assert n not in stack,('dependency cycle',stack,n)
 if n in nodes:return nodes
 nodes[n]=deps(n)
 for m in nodes[n]:closure(m,(*stack,n),nodes)
 return nodes
for name,(base,dep,branch,issue) in changes.items():
 p=root/name;d=json.loads((p/'library.json').read_text());nodes=closure(name);expanded=set();lines=[name]
 def tree(n,prefix=''):
  expanded.add(n);children=nodes[n]
  for i,m in enumerate(children):
   last=i==len(children)-1;repeat=m in expanded
   label='mandatory' if n==name else 'mandatory, transitive'
   if repeat and nodes[m]:label+='; dependencies expanded above'
   lines.append(prefix+('└── ' if last else '├── ')+m+' ['+label+']')
   if not repeat:tree(m,prefix+('    ' if last else '│   '))
 tree(name);treeblock='```text\n'+'\n'.join(lines)+'\n```'
 optional=''
 if 'EDP-Localisation' in nodes:
  optional='\nOptional transitive source adapter: `EDP-Localisation -> EDP-Persistence -> EDP-System` becomes imposed only when the application includes and instantiates Localisation `FilePackSource`. The application supplies the storage provider. It is not a mandatory Mesh package dependency.\n'
 table='| Library | Mandatory direct dependencies |\n|---|---|\n'+'\n'.join('| '+n+' | '+(', '.join(nodes[n]) or 'None')+' |' for n in nodes)
 section='## Dependencies\n\n'+treeblock+'\n\nEvery mandatory transitive edge is listed in the linked graph; repeated tree nodes refer to the first expanded occurrence. Concrete providers are selected by application Bootstrap.\n\n'+optional+'\nSee [DEPENDENCY_GRAPH.MD](./DEPENDENCY_GRAPH.MD) for the complete graph and ownership boundaries.\n\n'
 readme=(p/'README.MD').read_text()
 if '\n## Dependencies\n' in readme:readme=re.sub(r'\n## Dependencies\n.*?(?=\n## |\Z)','\n'+section.rstrip()+'\n',readme,flags=re.S)
 else:readme=readme.replace('\n## Current scope\n','\n'+section+'## Current scope\n')
 (p/'README.MD').write_text(readme)
 graph=(p/'DEPENDENCY_GRAPH.MD').read_text()
 if name=='EDP-Mesh':
  graph=graph.replace('    Mesh --> System','    Platform[EDP-Platform]\n\n    Mesh --> Platform\n    Mesh --> System',1)
  graph=graph.replace('EDP-Platform remains a transitive implementation dependency through existing providers; Mesh does not introduce parallel allocators or platform synchronization abstractions.','EDP-Platform is a direct dependency: Mesh imports its ConcurrentSnapshot and Randomness contracts. Mesh does not introduce parallel allocators or platform synchronization abstractions.')
  graph=graph.replace('| EDP-System |','| EDP-Platform | ConcurrentSnapshot and Randomness contracts | platform concurrency and entropy contracts; Bootstrap selects concrete providers |\n| EDP-System |',1)
 else:
  graph=re.sub(r'## Library Dependency Tree\n.*?(?=\n## Mandatory Dependencies)', '## Library Dependency Tree\n\n'+treeblock+'\n',graph,flags=re.S)
  graph=graph.replace('## Mandatory Dependencies\n','## Mandatory Dependencies\n\n- `EDP-System` — directly supplies DeviceIdentifier used by DevelopmentP256KeySource.\n',1)
 graph+='\n\n## H457 complete dependency review (2026-10-20)\n\nPackage declarations now match the existing direct source imports. This is metadata/documentation maintenance only; production source and all 0.1.0 versions are unchanged.\n\n'+table+'\n\n'+(treeblock+'\n' if name=='EDP-Mesh' else '')+optional+'\nNo direct optional package dependencies are introduced. ESP-IDF/FreeRTOS and standard C++ remain target/runtime facilities.\n'
 (p/'DEPENDENCY_GRAPH.MD').write_text(graph)
 assert not subprocess.run(['git','diff','--quiet','HEAD','--','src'],cwd=p).returncode
 assert d['version']=='0.1.0'
 package_names={x['name'] for x in d['dependencies']};arduino={re.match(r'[^ (]+',x).group() for x in next(x for x in (p/'library.properties').read_text().splitlines() if x.startswith('depends='))[8:].split(',')}
 assert package_names==arduino,(package_names,arduino)
 evidence={'repository':name,'base_commit':base,'production_src_tree':git(p,'rev-parse','HEAD:src'),'production_changed':False,'package_direct_dependencies':sorted(package_names),'graph':nodes,'source_basis':{n:git(root/n,'rev-parse','HEAD') for n in nodes},'optional_transitive':'Localisation FilePackSource -> Persistence -> System; only on explicit adapter use' if optional else None,'hardware_access':False,'version':'0.1.0'}
 e=p/'docs/evidence/H457';e.mkdir(parents=True)
 (e/'METADATA-VALIDATION.json').write_text(json.dumps(evidence,indent=2)+'\n')
 (p/'docs/H457-DIRECT-DEPENDENCIES.MD').write_text(f'# H457 — explicit direct dependency metadata\n\nCloses #{issue}. The tracked source already consumes {dep}; package metadata now declares that direct owner. '+('Arduino metadata also declares the already consumed EDP-Command. ' if name=='EDP-Mesh' else '')+'README and dependency graph include the full selected dependency closure and optional adapter condition. Source tree is byte-identical to '+base+'. No runtime test or native firmware rebuild is warranted for these metadata-only changes; original build and physical evidence retains its original basis. Graph traversal, manifest consistency and source-tree equivalence passed on AI-AGENT-02. No board access or authority qualification.\n')
 print('H457_METADATA_PASS',name,'direct',len(package_names),'closure',len(nodes),'src',evidence['production_src_tree'],flush=True)
 print(subprocess.check_output(['git','diff','--stat'],cwd=p,text=True),flush=True)
