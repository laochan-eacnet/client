<script setup lang="ts">
import { faCompactDisc, faDisplay, faLanguage, faLightbulb, faMicrochip, faVolumeHigh } from '@fortawesome/free-solid-svg-icons';
import { FontAwesomeIcon } from '@fortawesome/vue-fontawesome';
import { onMounted, ref, type Ref } from 'vue';
import { iidx, IIDXSoundMode } from '@/modules/iidx';
import router from '@/router';

const wasapiStatus = ref(-5);
const asioAvaliable: Ref<boolean> = ref(false)
const asioDeviceList: Ref<string[]> = ref([])

onMounted(async () => {
    await iidx.loadConfig();

    wasapiStatus.value = await window.laochan.checkWasapiDevice();
    const { devices, avaliable } = await window.laochan.getAsioDeviceList();

    asioDeviceList.value = devices;
    asioAvaliable.value = avaliable;
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
            <div class="item asio" :class="{ show: iidx.config.value?.soundMode == IIDXSoundMode.Asio }">
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
                    <option value="0">1080p 独占全屏</option>
                    <option value="3">1080p 窗口化</option>
                    <option value="2">900p 窗口化</option>
                    <option value="1">720p 窗口化</option>
                </select>
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
                    <FontAwesomeIcon :icon="faLightbulb"></FontAwesomeIcon>
                    2DX-GSM by aixxe
                </h3>
                <div class="flex justify-start align-center lh-100 py-1">
                    <input id="use-gsm" type="checkbox" v-bind:checked="iidx.config.value?.useGsm" @change="updateUseGsm">
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

.asio {
    transition: 0.3s ease;
    transform-origin: top;
    max-height: 10em;
    overflow: hidden;
    position: relative;
}

.asio:not(.show) {
    max-height: 0;
    opacity: 0;
    margin-bottom: 0;
    padding-top: 0;
    padding-bottom: 0;
    transform: translateY(-100px);
    z-index: -1;
}
</style>
