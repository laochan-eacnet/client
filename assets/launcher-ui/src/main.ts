import './assets/main.css'

import { createApp, ref } from 'vue'
import App from './App.vue'
import router from './router'
import { launcher } from './modules/launcher';
import { iidx } from './modules/iidx';
import { sdvx } from './modules/sdvx';
import { parseJsonText } from 'typescript';

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
        return await window.saucer.call<string[]>('detectGameInstall', [game]);
    },
    async detectGameInstall1(game: number) {
        return JSON.parse(await window.saucer.call<string>('detectGameInstall1', [game]));
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
        gameInfos: ref<GameMeta[]>([{
            game_type: 0,
            game_name: '',
            installed: false,
            install_path: '',
            resource_path: '',
            game_module_path: '',
            settings_module_path: '',
            updater_module_path: '',
            game_module_version: '',
            game_module_target_version: ''
        },{
            game_type: 0,
            game_name: '',
            installed: false,
            install_path: '',
            resource_path: '',
            game_module_path: '',
            settings_module_path: '',
            updater_module_path: '',
            game_module_version: '',
            game_module_target_version: ''
        },{
            game_type: 0,
            game_name: '',
            installed: false,
            install_path: '',
            resource_path: '',
            game_module_path: '',
            settings_module_path: '',
            updater_module_path: '',
            game_module_version: '',
            game_module_target_version: ''
        }]),
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
    for (let i = 0; i < 3; i++) {
        // window.laochan.ctx.gameInfos.value[i] = await window.laochan.detectGameInstall(i);
        window.laochan.ctx.gameInfos.value[i] = await window.laochan.detectGameInstall1(i);
    }
    launcher.loadConfig();
    iidx.loadConfig();
    sdvx.loadConfig();
})();

const app = createApp(App)
app.use(router)
app.mount('#app')
