import './assets/main.css'

import { createApp, ref } from 'vue'
import App from './App.vue'
import router from './router'

window.laochan = {
    close() {
        window.saucer.call('close', []);
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
    async readFile(path: string) {
        const result = await window.saucer.call<string>('readFile', [path]);
        if (result === '<NOT EXIST>') {
            return;
        }

        return result;
    },
    writeFile(path: string, content: string) {
        return  window.saucer.call<void>('writeFile', [path, content]);
    },
    async uuid() {
        return await window.saucer.call<string>('uuid', []);
    },
    ctx: {
        gamePaths: ref([[], [], []]),
    }
};

(async () => {
    for (let i = 0; i < 3; i++) {
        window.laochan.ctx.gamePaths.value[i] = await window.laochan.detectGameInstall(i);
    }
})();

const app = createApp(App)
app.use(router)
app.mount('#app')
