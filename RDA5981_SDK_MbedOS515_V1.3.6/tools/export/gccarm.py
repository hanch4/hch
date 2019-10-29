"""
mbed SDK
Copyright (c) 2011-2016 ARM Limited

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""
from os.path import splitext, basename, relpath, join, abspath, dirname
from os import curdir, getcwd
from tools.export.exporters import Exporter


class GccArm(Exporter):
    NAME = 'GccArm'
    TOOLCHAIN = 'GCC_ARM'

    TARGETS = [
        'LPC1768',
        'LPC1549',
        'KL05Z',
        'KL25Z',
        'KL43Z',
        'KL46Z',
        'K64F',
        'K22F',
        'K20D50M',
        'LPC4088',
        'LPC4088_DM',
        'LPC4330_M4',
        'LPC11U24',
        'LPC1114',
        'LPC11U35_401',
        'LPC11U35_501',
        'LPC11U37H_401',
        'LPC810',
        'LPC812',
        'LPC824',
        'SSCI824',
        'STM32F407',
        'DISCO_F100RB',
        'DISCO_F051R8',
        'DISCO_F407VG',
        'DISCO_F429ZI',
        'DISCO_F469NI',
        'DISCO_F303VC',
        'DISCO_F746NG',
        'DISCO_F769NI',
        'DISCO_L476VG',
        'UBLOX_C027',
        'ARCH_PRO',
        'NRF51822',
        'HRM1017',
        'TY51822R3',
        'RBLAB_NRF51822',
        'RBLAB_BLENANO',
        'LPC2368',
        'LPC2460',
        'LPCCAPPUCCINO',
        'ARCH_BLE',
        'MTS_GAMBIT',
        'ARCH_MAX',
        'NUCLEO_F401RE',
        'NUCLEO_F410RB',
        'NUCLEO_F411RE',
        'NUCLEO_F429ZI',
        'NUCLEO_F446RE',
        'NUCLEO_F446ZE',
        'B96B_F446VE',
        'ARCH_MAX',
        'NUCLEO_F030R8',
        'NUCLEO_F031K6',
        'NUCLEO_F042K6',
        'NUCLEO_F070RB',
        'NUCLEO_F072RB',
        'NUCLEO_F091RC',
        'NUCLEO_F103RB',
        'NUCLEO_F207ZG',
        'NUCLEO_F302R8',
        'NUCLEO_F303K8',
        'NUCLEO_F303RE',
        'NUCLEO_F334R8',
        'NUCLEO_F303ZE',
        'NUCLEO_F746ZG',
        'NUCLEO_F767ZI',
        'DISCO_L053C8',
        'NUCLEO_L011K4',
        'NUCLEO_L031K6',
        'NUCLEO_L053R8',
        'NUCLEO_L073RZ',
        'NUCLEO_L432KC',
        'NUCLEO_L476RG',
        'DISCO_F334C8',
        'MAX32600MBED',
        'MAXWSNENV',
        'MAX32620HSP',
        'MTS_MDOT_F405RG',
        'MTS_MDOT_F411RE',
        'NUCLEO_L152RE',
        'NRF51_DK',
        'NRF51_DONGLE',
        'NRF51_MICROBIT',
        'MTM_MTCONNECT04S',
        'SEEED_TINY_BLE',
        'DISCO_F401VC',
        'DELTA_DFCM_NNN40',
        'RZ_A1H',
        'MOTE_L152RC',
        'EFM32WG_STK3800',
        'EFM32LG_STK3600',
        'EFM32GG_STK3700',
        'EFM32ZG_STK3200',
        'EFM32HG_STK3400',
        'EFM32PG_STK3401',
        'NZ32_SC151',
        'SAMR21G18A',
        'TEENSY3_1',
        'SAMD21J18A',
        'SAMD21G18A',
        'SAML21J18A',
        'SAMG55J19',
        'ARM_BEETLE_SOC',
        'ELMO_F411RE',
        'BLUEPILL_F103C8',
    ]

    DOT_IN_RELATIVE_PATH = True

    MBED_CONFIG_HEADER_SUPPORTED = True

    def generate(self):
        # "make" wants Unix paths
        self.resources.win_to_unix()

        to_be_compiled = []
        for r_type in ['s_sources', 'c_sources', 'cpp_sources']:
            r = getattr(self.resources, r_type)
            if r:
                for source in r:
                    base, ext = splitext(source)
                    to_be_compiled.append(base + '.o')

        libraries = []
        for lib in self.resources.libraries:
            l, _ = splitext(basename(lib))
            libraries.append(l[3:])

        ctx = {
            'name': self.project_name,
            'to_be_compiled': to_be_compiled,
            'object_files': self.resources.objects,
            'include_paths': self.resources.inc_dirs,
            'library_paths': self.resources.lib_dirs,
            'linker_script': self.resources.linker_script,
            'libraries': libraries,
            'symbols': self.toolchain.get_symbols(),
            'cpu_flags': self.toolchain.cpu,
            'hex_files': self.resources.hex_files,
            'vpath': (["../../.."]
                      if basename(dirname(dirname(self.export_dir))) == "projectfiles"
                      else [".."])
        }

        for key in ['include_paths', 'library_paths', 'linker_script', 'hex_files']:
            if isinstance(ctx[key], list):
                ctx[key] = [ctx['vpath'][0] + "/" + t for t in ctx[key]]
            else:
                ctx[key] = ctx['vpath'][0] + "/" + ctx[key]
        if "../." not in ctx["include_paths"]:
            ctx["include_paths"] += ['../.']
        ctx.update(self.flags)
        self.gen_file('gcc_arm_%s.tmpl' % self.target.lower(), ctx, 'Makefile')
