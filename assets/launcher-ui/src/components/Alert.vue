<script setup lang="ts">
import { ref } from 'vue';

const message = ref('');
const color = ref('');
const element: Ref<HTMLElement | undefined> = ref(undefined);

let timer: number | undefined = undefined;

window.laochan.alert.__cb = (msg, colour, timeout) => {
    if (!element.value) return;

    clearTimeout(timer);

    color.value = colour;
    message.value = msg;
    element.value.classList.remove('out');
    element.value.classList.add('in');

    timer = setTimeout(() => {
        if (!element.value) return;

        element.value.classList.remove('in');
        element.value.classList.add('out');

        clearTimeout(timer);
        timer = undefined;
    }, timeout);
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
    animation-duration: 0.5s;
    animation-direction: normal;
    animation-fill-mode: both;
    animation-timing-function: ease;
}

.out {
    animation-name: out;
    animation-duration: 0.5s;
    animation-direction: normal;
    animation-fill-mode: both;
    animation-timing-function: ease;
}
</style>
