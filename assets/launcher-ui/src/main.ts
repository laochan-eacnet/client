import './assets/main.css'

import { createApp } from 'vue'
import App from './App.vue'
import router from './router'

window.saucer ??= {
    _idc: 0,
    _rpc: [],
    window_edge:
    {
        top: 1,
        bottom: 2,
        left: 4,
        right: 8,
    },
    on_message: async (message: string) => {
        window.chrome.webview.postMessage(message);
    },
    start_drag: async () => {
        await window.saucer.on_message(JSON.stringify({
            ["saucer:drag"]: true
        }));
    },
    start_resize: async (edge) => {
        await window.saucer.on_message(JSON.stringify({
            ["saucer:resize"]: true,
            edge,
        }));
    },
    call: async (name, params) =>
    {
        if (!Array.isArray(params))
        {
            throw 'Bad Arguments, expected array';
        }

        if (typeof name !== 'string')
        {
            throw 'Bad Name, expected string';
        }

        const id = ++window.saucer._idc;
        
        const rtn = new Promise((resolve, reject) => {
            window.saucer._rpc[id] = {
                reject,
                resolve,
            };
        });

        await window.saucer.on_message(JSON.stringify({
                id,
                name,
                params,
        }));

        return rtn;
    },
    _resolve : async (id, value) => {
        await window.saucer.on_message(JSON.stringify({
                id,
                result: value === undefined ? null : value,
        }));
    },
};

const app = createApp(App)

app.use(router)

app.mount('#app')
