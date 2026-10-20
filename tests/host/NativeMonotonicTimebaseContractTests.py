#!/usr/bin/env python3
"""Actual native provider with explicit SDK MODEL targets/configs; no device use."""
from pathlib import Path
import argparse,subprocess,tempfile,shutil,json
root=Path(__file__).resolve().parents[2]
p=argparse.ArgumentParser();p.add_argument('--clock',type=Path,default=root.parent/'EDP-Clock');p.add_argument('--platform',type=Path,default=root.parent/'EDP-Platform');p.add_argument('--system',type=Path,default=root.parent/'EDP-System');p.add_argument('--mode',choices=['normal','sanitize','analyze','release'],default='normal');p.add_argument('--results',type=Path);args=p.parse_args()
base={'ESP_PLATFORM':1,'CONFIG_XTAL_FREQ':40,'CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ':160}
esp32={'CONFIG_IDF_TARGET_ESP32':1,'CONFIG_ESP_TIMER_IMPL_TG0_LAC':1}
s3={'CONFIG_IDF_TARGET_ESP32S3':1,'CONFIG_ESP_TIMER_IMPL_SYSTIMER':1}
cases=[('esp32',esp32,0),('s3',s3,0),('esp32_cpu240',{**esp32,'CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ':240},0),('unsupported_target',{'CONFIG_IDF_TARGET_ESP32C3':1,'CONFIG_ESP_TIMER_IMPL_SYSTIMER':1},1),('conflicting_targets',{**esp32,**s3},1),('unsupported_sdk',{**esp32,'EDP_MODEL_IDF_VERSION':0x050504},2),('crystal26',{**esp32,'CONFIG_XTAL_FREQ':26},3),('missing_crystal',{**esp32,'CONFIG_XTAL_FREQ':None},3),('cpu80',{**esp32,'CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ':80},3),('power_management',{**esp32,'CONFIG_PM_ENABLE':1},3),('tickless',{**esp32,'CONFIG_FREERTOS_USE_TICKLESS_IDLE':1},3),('wrong_timer',{'CONFIG_IDF_TARGET_ESP32':1,'CONFIG_ESP_TIMER_IMPL_SYSTIMER':1},3)]
common=['-std=c++20','-Wall','-Wextra','-Werror','-pedantic','-I'+str(root/'tests/host/include'),'-I'+str(root/'src'),'-I'+str(args.clock/'src'),'-I'+str(args.platform/'src'),'-I'+str(args.system/'src')]
rows=[]
with tempfile.TemporaryDirectory(prefix='h467-native-timebase-model-') as temp:
 for cc in (['clang++'] if args.mode=='analyze' else ['g++','clang++']):
  if not shutil.which(cc):raise SystemExit('Missing compiler '+cc)
  for name,flags,want in cases:
   options={**base,**flags,'EDP_MODEL_EXPECTED_BUILD_RESULT':want};defines=['-D'+k+'='+str(v) for k,v in options.items() if v is not None]
   exe=Path(temp)/(cc.replace('+','p')+'-'+name)
   modeflags=['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie'] if args.mode=='sanitize' else ['-O2','-DNDEBUG','-UNDEBUG'] if args.mode=='release' else ['--analyze','-Xanalyzer','-analyzer-output=text'] if args.mode=='analyze' else ['-O2']
   command=[cc,*common,*modeflags,*defines,str(root/'tests/host/NativeMonotonicTimebaseHostTests.cpp')]
   if args.mode!='analyze':command+=['-o',str(exe)]
   subprocess.run(command,check=True)
   if args.mode!='analyze':subprocess.run([str(exe)],check=True)
   rows.append({'compiler':cc,'case':name,'mode':args.mode,'expected_build_result':want,'command':command,'hardware':False})
print('H467_NATIVE_TIMEBASE_MODEL_'+args.mode.upper()+'_PASS|variants='+str(len(rows))+'|hardware=0|physical_rate_measured=0')
if args.results:args.results.write_text(json.dumps(rows,indent=2)+'\n')
