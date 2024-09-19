<script setup lang="ts">
import { faHdd, faIdCard, faInfo, faServer, faTape, faTerminal, faWarning } from '@fortawesome/free-solid-svg-icons';
import { FontAwesomeIcon } from '@fortawesome/vue-fontawesome';
import { onMounted, ref, type Ref, getCurrentInstance } from 'vue';
import { launcher } from '@/modules/launcher';
import { faSteam } from '@fortawesome/free-brands-svg-icons';

const gameNames = [
    'beatmania IIDX INFINITAS',
    'SOUND VOLTEX EXCEED GEAR',
    'GITADORA',
]

function openPath(path: string) {
    return window.saucer.call('shellExecute', [path.replaceAll('\\', '/')]);
}

function pathes() {
    return window.laochan.ctx.gameInfos.value;
}

function revealToken(e: FocusEvent) {
    (e.target as HTMLInputElement).type = 'text';
}

function hideToken(e: FocusEvent) {
    (e.target as HTMLInputElement).type = 'password';
}

function updateToken(e: Event) {
    if (!launcher.config.value) {
        return;
    }

    launcher.config.value.token = (e.target as HTMLInputElement).value;
}

function updateServerUrl(e: Event) {
    if (!launcher.config.value) {
        return;
    }

    launcher.config.value.serverUrl = (e.target as HTMLInputElement).value;
}

function updateEnableConsole(e: Event) {
    if (!launcher.config.value) {
        return;
    }

    launcher.config.value.enableConsole = (e.target as HTMLInputElement).checked;
}

function updateEnableSteamOverlay(e: Event) {
    if (!launcher.config.value) {
        return;
    }

    launcher.config.value.enableSteamOverlay = (e.target as HTMLInputElement).checked;
}

async function save() {
    await launcher.saveConfig();
    window.laochan.alert.show('已保存启动器设置', '#40B681', 2000);
}
</script>

<template>
    <div class="page">
        <div class="container">
            <h2>
                <FontAwesomeIcon :icon="faTape"></FontAwesomeIcon>
                佬缠网设置
            </h2>
            <div class="item">
                <div class="flex">
                    <h3>
                        <FontAwesomeIcon :icon="faIdCard"></FontAwesomeIcon>
                        登入令牌
                    </h3>
                    <div>
                        <button class="btn link" @click="launcher.resetToken">重设为机器码</button>
                    </div>
                </div>
                <input class="text-input" type="password" v-bind:value="launcher.config.value?.token" @focus="revealToken"
                    @blur="hideToken" @input="updateToken">
            </div>
            <div class="item">
                <div class="flex">
                    <h3>
                        <FontAwesomeIcon :icon="faServer"></FontAwesomeIcon>
                        Bootstrap 地址
                    </h3>
                    <div>
                        <button class="btn link" @click="launcher.resetServerUrl">重设为默认地址</button>
                    </div>
                </div>
                <input class="text-input" type="text" v-bind:value="launcher.config.value?.serverUrl" @input="updateServerUrl">
            </div>
            <div class="item">
                <h3>
                    <FontAwesomeIcon :icon="faTerminal"></FontAwesomeIcon>
                    调试控制台
                </h3>
                <div class="flex justify-start align-center lh-100 py-1">
                    <input id="use-console" type="checkbox" v-bind:checked="launcher.config.value?.enableConsole" @change="updateEnableConsole">
                    <label for="use-console">启用调试控制台</label>
                </div>
                <small><FontAwesomeIcon :icon="faWarning"></FontAwesomeIcon>关闭调试控制台可能可以缓解性能问题</small>
            </div>
            <div class="item">
                <h3>
                    <FontAwesomeIcon :icon="faSteam"></FontAwesomeIcon>
                    Steam 游戏内覆盖
                </h3>
                <div class="flex justify-start align-center lh-100 py-1">
                    <input id="use-steam-overlay" type="checkbox" v-bind:checked="launcher.config.value?.enableSteamOverlay" @change="updateEnableSteamOverlay">
                    <label for="use-steam-overlay">启用 Steam 游戏内覆盖</label>
                </div>
                <small><FontAwesomeIcon :icon="faWarning"></FontAwesomeIcon>关闭 Steam 游戏内覆盖可能可以缓解性能问题</small>
            </div>
            <div class="flex">
                <div></div>
                <button class="btn primary" @click="save">保存设置</button>
            </div>
            <hr>
            <h2>
                <FontAwesomeIcon :icon="faHdd"></FontAwesomeIcon>
                游戏安装详情
            </h2>
            <div v-for="name, i in gameNames" class="item">
                <h2>{{ name }}</h2>
                <div v-if="pathes()[i].installed">
                    <h3>安装路径: <a class="path link" @click="openPath(pathes()[i].install_path)">{{ pathes()[i].install_path
                            }}</a></h3>
                    <h3>资源路径: <a class="path link" @click="openPath(pathes()[i].resource_path)">{{ pathes()[i].resource_path
                            }}</a></h3>
                    <h3>游戏版本: <a class="path link">{{ pathes()[i].game_module_version
                            }}</a></h3>
                    <h3>支持版本: <a class="path link">{{ pathes()[i].game_module_target_version
                            }}</a></h3>
                </div>
                <div v-else>
                    <h3 class="gray">未安装</h3>
                </div>
            </div>
            <footer>
                LAOCHAN EACNET IS A MOD, NOT AFFILIATED WITH KONAMI<br>
                WE DON'T SUPPORT PIRATED GAME<br>
                PLEASE SUBSCRIPT TO THE GAME COURSE
            </footer>
        </div>
    </div>
</template>
