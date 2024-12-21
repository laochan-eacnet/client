<script setup lang="ts">
import { faCompactDisc, faComputer, faDisplay, faHardDrive, faLanguage, faLightbulb, faMicrochip, faTelevision, faUpRightAndDownLeftFromCenter, faVolumeHigh } from '@fortawesome/free-solid-svg-icons';
import { FontAwesomeIcon } from '@fortawesome/vue-fontawesome';
import { onMounted, ref, type Ref } from 'vue';
import { iidx, IIDXDisplayMode, IIDXSoundMode } from '@/modules/iidx';

interface CombinedDisplayMode {
    width: number;
    height: number;
    rates: number[];
    good: boolean;
};

const wasapiStatus = ref(-5);
const asioAvaliable: Ref<boolean> = ref(false)
const asioDeviceList: Ref<string[]> = ref([])
const displayModes: Ref<CombinedDisplayMode[]> = ref([])

const has60hz: Ref<boolean> = ref(false)
const has120hz: Ref<boolean> = ref(false)
const hasHighRefreshRate: Ref<boolean> = ref(false)
const resIndex: Ref<number> = ref(0)

onMounted(async () => {
    await iidx.loadConfig();

    wasapiStatus.value = await window.laochan.checkWasapiDevice();
    const { devices, avaliable } = await window.laochan.getAsioDeviceList();

    asioDeviceList.value = devices;
    asioAvaliable.value = avaliable;

    const modes = (await window.laochan.queryDisplayModes())
        .sort((a, b) => {
            return a.hz - b.hz;
        });

    const combinedModes: CombinedDisplayMode[] = [];

    for (const mode of modes) {
        let combined = combinedModes.find(v => v.height == mode.height && v.width == mode.width);
        if (!combined) {
            combined = {
                height: mode.height,
                width: mode.width,
                good: false,
                rates: [],
            };

            combinedModes.push(combined);
        }

        if (mode.hz > 59) combined.rates.push(mode.hz);

        const is16by9 = Math.abs(mode.width / mode.height - 1.777777777777778) < 0.01;

        if (Math.abs(mode.hz - 119) <= 1) {
            has120hz.value = true;
            if (is16by9) {
                combined.good = true;
            }

            continue;
        }

        if (Math.abs(mode.hz - 59) <= 1) {
            has60hz.value = true;
            if (is16by9) {
                combined.good = true;
            }

            continue;
        }
    }

    displayModes.value = combinedModes
        .map(v => {
            v.rates.sort((a, b) => b - a);
            return v;
        })
        .sort((a, b) => {
            if (a.good && !b.good)
                return -1;

            if (!a.good && b.good)
                return 1;

            return b.width * b.height - a.width * a.height;
        });

    resIndex.value = displayModes.value.findIndex(v => v.width == iidx.config.value?.resolution.w && v.height == iidx.config.value?.resolution.h);
    if (resIndex.value == -1) resIndex.value = 0;
});

function updateSoundMode(e: Event) {
    if (!iidx.config.value) {
        return;
    }

    iidx.config.value.soundMode = parseInt((e.target as HTMLInputElement).value);
}

function updateAsioDevice(e: Event) {
    if (!iidx.config.value) {
        return;
    }

    iidx.config.value.asioDevice = (e.target as HTMLInputElement).value;
}

function updateDisplayMode(e: Event) {
    if (!iidx.config.value) {
        return;
    }

    iidx.config.value.displayMode = parseInt((e.target as HTMLInputElement).value);
}

function updateGraphicsAPI(e: Event) {
    if (!iidx.config.value) {
        return;
    }

    iidx.config.value.graphicsAPI = parseInt((e.target as HTMLInputElement).value);
}

function updateResoltion(e: Event) {
    if (!iidx.config.value) {
        return;
    }
    resIndex.value = parseInt((e.target as HTMLInputElement).value);
    const resolution = displayModes.value[resIndex.value];

    iidx.config.value.resolution.w = resolution.width;
    iidx.config.value.resolution.h = resolution.height;
}

function updateLanguage(e: Event) {
    if (!iidx.config.value) {
        return;
    }

    iidx.config.value.language = parseInt((e.target as HTMLInputElement).value);
}

function updateUseGsm(e: Event) {
    if (!iidx.config.value) {
        return;
    }

    iidx.config.value.useGsm = (e.target as HTMLInputElement).checked;
}

function updateAcDataPath(e: Event) {
    if (!iidx.config.value) {
        return;
    }

    iidx.config.value.acDataPath = (e.target as HTMLInputElement).value
        .replaceAll('\\', '/');;
}

async function save() {
    await iidx.saveConfig();
    window.laochan.alert.show('已保存 IIDX 设置', '#40B681', 2000);
}
</script>

<template>
    <div class="page">
        <div class="container">
            <h2>
                <FontAwesomeIcon :icon="faCompactDisc"></FontAwesomeIcon>
                INFINITAS 额外设置
            </h2>
            <div class="item">
                <h3>
                    <FontAwesomeIcon :icon="faVolumeHigh"></FontAwesomeIcon>
                    音频输出模式
                </h3>
                <select class="text-input" v-bind:value="iidx.config.value?.soundMode" @change="updateSoundMode">
                    <option value="0">WASAPI</option>
                    <option value="1">ASIO</option>
                </select>
                <div v-if="wasapiStatus == 1">警告: 当前选择的音频设备不支持 44100 Hz 模式输出, 可能无法正常以 WASAPI 独占模式启动, 请前往<a
                        @click="iidx.settings">游戏设置</a>关闭
                    WASAPI 独占模式或切换输出设备。</div>
                <div v-if="wasapiStatus < 0">警告: WASAPI 测试失败 ({{ wasapiStatus }}), 游戏很有可能无法启动。</div>
            </div>
            <div class="item optional" :class="{ show: iidx.config.value?.soundMode == IIDXSoundMode.Asio }">
                <h3>
                    <FontAwesomeIcon :icon="faMicrochip"></FontAwesomeIcon>
                    ASIO 输出设备
                </h3>
                <select class="text-input" v-bind:value="iidx.config.value?.asioDevice" @change="updateAsioDevice">
                    <option v-for="device in asioDeviceList" :value="device">{{ device }}</option>
                </select>
                <div v-if="!asioAvaliable">警告: 未检测到有效的 ASIO 输出设备, 游戏可能无法正常启动。</div>
            </div>
            <div class="item">
                <h3>
                    <FontAwesomeIcon :icon="faDisplay"></FontAwesomeIcon>
                    显示模式
                </h3>
                <select class="text-input" v-bind:value="iidx.config.value?.displayMode" @change="updateDisplayMode">
                    <option value="0">独占全屏</option>
                    <option value="1">无边框窗口</option>
                    <option value="2">窗口化</option>
                </select>
            </div>
            <div class="item">
                <h3>
                    <FontAwesomeIcon :icon="faComputer"></FontAwesomeIcon>
                    图形API
                </h3>
                <select class="text-input" v-bind:value="iidx.config.value?.graphicsAPI" @change="updateGraphicsAPI">
                    <option value="0">原生D3D9</option>
                    <option value="1">D3D9On12</option>
                    <option value="2">DXVK</option>
                </select>
            </div>
            <div class="item optional" :class="{ show: iidx.config.value?.displayMode != IIDXDisplayMode.BorderlessWindowed }">
                <h3>
                    <FontAwesomeIcon :icon="faUpRightAndDownLeftFromCenter"></FontAwesomeIcon>
                    显示分辨率
                </h3>
                <select class="text-input" v-bind:value="resIndex" @change="updateResoltion">
                    <option v-for="mode, i in displayModes" :value="i">
                        <template v-if="!mode.good">(不推荐)</template>
                        {{ mode.width }}x{{ mode.height }} (
                            <template v-for="rate, j in mode.rates">{{ rate }}
                                hz<template v-if="j !== mode.rates.length - 1">, </template>
                            </template>
                        )
                    </option>
                </select>
                <div v-if="!has120hz && hasHighRefreshRate">警告: 显示器不支持 120 Hz 高刷模式, 请使用显卡驱动面板或者 CRU 添加 120 Hz 的刷新率模式,
                    否则游戏将不会以高刷新率模式启动。</div>
                <div v-if="!has60hz && !has120hz && !hasHighRefreshRate">警告: 显示器不支持游戏所需刷新率 (60Hz/120Hz),
                    游戏可能出现音画不同步现象或无法启动。</div>
            </div>
            <div class="item">
                <h3>
                    <FontAwesomeIcon :icon="faLanguage"></FontAwesomeIcon>
                    游戏语言
                </h3>
                <select class="text-input" v-bind:value="iidx.config.value?.language" @change="updateLanguage">
                    <option value="0">日本語</option>
                    <option value="1">English</option>
                    <option value="2">한국어</option>
                </select>
            </div>
            <div class="item">
                <h3>
                    <FontAwesomeIcon :icon="faHardDrive"></FontAwesomeIcon>
                    AC 数据挂载路径
                </h3>
                <input class="text-input" type="text" v-bind:value="iidx.config.value?.acDataPath"
                    @input="updateAcDataPath">
                <div>仅支持 PINKY CRUSH 数据, 请指向 data 目录</div>
            </div>
            <div class="item">
                <h3>
                    <FontAwesomeIcon :icon="faLightbulb"></FontAwesomeIcon>
                    2DX-GSM by aixxe
                </h3>
                <div class="flex justify-start align-center lh-100 py-1">
                    <input id="use-gsm" type="checkbox" v-bind:checked="iidx.config.value?.useGsm"
                        @change="updateUseGsm">
                    <label for="use-gsm">加载 2DX-GSM 模块</label>
                </div>
            </div>
            <div class="flex">
                <div></div>
                <button class="btn primary" @click="save">保存设置</button>
            </div>
        </div>
        <div class="background"></div>
    </div>
</template>

<style scoped lang="scss">
.page {
    margin-top: 48px;
    min-height: calc(100vh - 48px);
    position: relative;
    padding-bottom: 2em;
}

.page>.background {
    position: fixed;
    background-image: url(@/assets/iidx.jpg);
    background-size: contain;
    background-position: center;
    top: -2.5%;
    left: -2.5%;
    width: 105%;
    height: 105%;
    filter: brightness(0.5);
    z-index: -2;
}

.optional {
    transition: 0.3s ease;
    transform-origin: top;
    max-height: 10em;
    overflow: hidden;
    position: relative;
}

.optional:not(.show) {
    max-height: 0;
    opacity: 0;
    margin-bottom: 0;
    padding-top: 0;
    padding-bottom: 0;
    transform: translateY(-100px);
    z-index: -1;
}
</style>
