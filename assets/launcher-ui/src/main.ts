import './assets/main.css'

import { createApp, ref } from 'vue'
import App from './App.vue'
import router from './router'
import { launcher } from './modules/launcher';
import { iidx } from './modules/iidx';
import { sdvx } from './modules/sdvx';
import { parseJsonText } from 'typescript';
import { gitadora } from './modules/gitadora';

window.laochan = {
    close() {
        window.saucer.call('close', []);
    },
    version() {
        return window.saucer.call<string>('version', []);
    },
    minimize() {
        window.saucer.call('minimize', []);
    },
    mounted() {
        window.saucer.call('mounted', []);
    },
    shellExecute(file: string, args: string = ''): void {
        window.saucer.call('shellExecute', [file, args]);
    },
    async detectGameInstall(game: number) {
        return JSON.parse(await window.saucer.call<string>('detectGameInstall', [game]));
    },
    async readFile(path: string) {
        const result = await window.saucer.call<string>('readFile', [path]);
        if (result === '<NOT EXIST>') {
            return;
        }

        return result;
    },
    writeFile(path: string, content: string) {
        return window.saucer.call<void>('writeFile', [path, content]);
    },
    async uuid() {
        return await window.saucer.call<string>('uuid', []);
    },
    setGame(game: number) {
        return window.saucer.call<void>('setGame', [game]);
    },
    setParam(key: string, value: string) {
        return window.saucer.call<void>('setParam', [key, value]);
    },
    async getAsioDeviceList() {
        const devices = await window.saucer.call<string[]>('getAsioDeviceList', []);
        const avaliable = !!devices.length;
        if (!avaliable) {
            devices.push('XONAR SOUND CARD(64)');
        }

        return { devices, avaliable };
    },
    checkWasapiDevice() {
        return window.saucer.call<number>('checkWasapiDeviceStatus', []);
    },
    queryDisplayModes() {
        return window.saucer.call<string[]>('queryDisplayModes', [])
            .then(modes => modes.map(v => {
                const [width, height, hz] = JSON.parse(v);
                return { width, height, hz };
            }));
    },
    selfPath: () => {
        return window.saucer.call<string>('selfPath', []);
    },
    num: () => {},
    ctx: {
        gameInfos: ref<GameMeta[]>(new Array<GameMeta>(3)),
    },
    alert: {
        __cb: undefined,
        show(message, color, timeout) {
            if (!this.__cb) {
                return;
            }

            this.__cb(message, color, timeout);
        }
    },
};

(async () => {

    await launcher.loadConfig();
    await iidx.loadConfig();
    await sdvx.loadConfig();
    await iidx.UpdateMeta();
    await sdvx.UpdateMeta();
    await gitadora.UpdateMeta();
})();

const app = createApp(App)
app.use(router)
app.mount('#app')