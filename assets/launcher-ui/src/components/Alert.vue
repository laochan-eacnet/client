<script setup lang="ts">
import { ref } from 'vue';

const message = ref('');
const color = ref('');
const element: Ref<HTMLElement | undefined> = ref(undefined);

const queue: { msg: string, colour: string, timeout: number }[] = [];
let timer: number | undefined = undefined;

function show(data: { msg: string, colour: string, timeout: number }) {
    if (!element.value) return;

    clearTimeout(timer);

    color.value = data.colour;
    message.value = data.msg;
    element.value.classList.remove('out');
    element.value.classList.add('in');

    timer = setTimeout(() => {
        if (!element.value) return;

        element.value.classList.remove('in');
        element.value.classList.add('out');

        clearTimeout(timer);
        
        // wait for out animation
        setTimeout(() => {
            timer = undefined;

            const next = queue.shift();
            if (next) show(next);
        }, 250);
    }, data.timeout);
};

window.laochan.alert.__cb = (msg, colour, timeout) => {
    if (timer === undefined)
        show({ msg, colour, timeout });
    else
        queue.push({ msg, colour, timeout });
};
</script>

<template>
    <div ref="element" class="alert">
        <div class="content" :style="{ 'background-color': color }">
            {{ message }}
        </div>
    </div>
</template>

<style lang="scss">
.alert {
    position: fixed;
    z-index: 20;
    top: 5em;
    width: 100vw;
    display: flex;
    max-height: 2em;
}

.alert>.content {
    margin: auto;
    text-align: center;
    padding: 0.5em 1em;
    border-radius: 5px;
}

@keyframes in {
    0% {
        opacity: 0;
        max-height: 0;
        transform: translateX(-100vw) scale(0.8);
    }

    100% {}
}

@keyframes out {
    100% {
        opacity: 0;
        max-height: 0;
        transform: translateX(50vw) scale(0.8);
    }

    0% {}
}

.in {
    animation-name: in;
    animation-duration: 0.25s;
    animation-direction: normal;
    animation-fill-mode: both;
    animation-timing-function: ease;
}

.out {
    animation-name: out;
    animation-duration: 0.25s;
    animation-direction: normal;
    animation-fill-mode: both;
    animation-timing-function: ease;
}
</style>
