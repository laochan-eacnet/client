<script setup lang="ts">
import { faHdd, faIdCard, faTape } from '@fortawesome/free-solid-svg-icons';
import { FontAwesomeIcon } from '@fortawesome/vue-fontawesome';
import { onMounted, ref, type Ref, getCurrentInstance } from 'vue';

const gameNames = [
    'beatmania IIDX INFINITAS',
    'SOUND VOLTEX EXCEED GEAR',
    'GITADORA',
]

interface LauncherConfig {
    token: string;
}

const config: Ref<LauncherConfig> = ref({} as LauncherConfig)

function openPath(path: string) {
    return window.saucer.call('shellExecute', [path.replaceAll('\\', '/')]);
}

function pathes() {
    return window.laochan.ctx.gamePaths.value;
}

async function resetToken() {
    config.value['token'] = await window.laochan.uuid();
}

function revealToken(e: FocusEvent) {
    (e.target as HTMLInputElement).type = 'text';
}

function hideToken(e: FocusEvent) {
    (e.target as HTMLInputElement).type = 'password';
}

async function saveConfig() {
    await window.laochan.writeFile('laochan-config.json', JSON.stringify(config.value));
}

function updateToken(e: Event) {
    config.value['token'] = (e.target as HTMLInputElement).value;
}

onMounted(async () => {
    const configJson = await window.laochan.readFile('laochan-config.json');
    if (configJson) {
        config.value = JSON.parse(configJson);
    }

    if (!configJson || !config.value.token) {
        await resetToken();
        await saveConfig();
    }
});
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
                        <button class="btn link" @click="resetToken">重设为机器码</button>
                    </div>
                </div>
                <input class="text-input" type="password" v-bind:value="config.token" @focus="revealToken"
                    @blur="hideToken" @input="updateToken">
            </div>
            <div class="flex">
                <div></div>
                <button class="btn primary" @click="saveConfig">保存设置</button>
            </div>
            <hr>
            <h2>
                <FontAwesomeIcon :icon="faHdd"></FontAwesomeIcon>
                游戏安装详情
            </h2>
            <div v-for="name, i in gameNames" class="item">
                <h2>{{ name }}</h2>
                <div v-if="pathes()[i].length">
                    <h3>安装路径: <a class="path link" @click="openPath(pathes()[i][0])">{{ pathes()[i][0]
                            }}</a></h3>
                    <h3>资源路径: <a class="path link" @click="openPath(pathes()[i][1])">{{ pathes()[i][1]
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

<style scoped>
svg.svg-inline--fa {
    font-size: 90%;
    margin-right: 8px;
}

.page {
    width: 100vw;
    margin-top: 48px;
    height: calc(100vh - 48px);
    overflow-y: scroll;
    line-height: 250%;
    font-size: 90%;
}

.gray {
    color: #666;
}

.path {
    user-select: text;
    cursor: pointer;
}

.container {
    margin: auto;
    margin-top: 64px;

    width: 960px;
}

hr {
    border-width: 0;
    height: 1px;
    background-color: #808080;
}

.container>* {
    margin-bottom: 16px;
}

.container>.item {
    background-color: #121212;
    padding: 1em 2em;
    border-radius: 5px;
}

footer {
    margin-top: 2em;
    padding-bottom: 2em;
    text-align: center;
    line-height: 150%;
}

.text-input {
    display: block;
    border: none;
    border-bottom: 1px solid gray;
    background-color: #0C0C0C;
    padding: 0.5em;
    width: 100%;
    margin-top: 0.5em;
    margin-bottom: 0.5em;
    outline: none;

}

.text-input:focus {
    border-bottom: 1px solid white;
}

.btn {
    appearance: none;
    outline: none;
    border: none;
    display: block;
    border-radius: 5px;
    background-color: transparent;
    padding: 0.5em 1em;
    position: relative;
    overflow: hidden;
    transition: 0.2s ease;
    cursor: pointer;
}

.btn.primary {
    background-color: rgb(34, 161, 51);
    padding: 1em 2em;
}

.btn:hover {
    filter: brightness(1.5);
    transform: scale(1.05) translateY(-2px);
}

.btn.primary:hover {
    box-shadow: 0 2px 2px black;
}

.btn:active {
    filter: brightness(0.8);
    transform: scale(0.99) translateY(2px);
}


.flex {
    display: flex;
    justify-content: space-between;
    align-items: baseline;
}
</style>
