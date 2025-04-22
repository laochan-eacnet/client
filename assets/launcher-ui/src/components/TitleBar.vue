<script setup lang="ts">
import { onMounted, ref, type Ref } from 'vue';
import { RouterLink } from 'vue-router';

import IconClose from '@/components/icons/IconClose.vue';
import IconSettings from '@/components/icons/IconSettings.vue';
import IconMinimize from '@/components/icons/IconMinimize.vue';
import IconBack from '@/components/icons/IconBack.vue';

defineProps<{
    title: string
}>()

const titleBar: Ref<HTMLElement | undefined> = ref();

onMounted(() => {
    if (!titleBar.value) {
        console.error('titlebar element not found')
        return;
    }

    titleBar.value.addEventListener('mousedown', () => window.saucer.start_drag());

    setTimeout(() => {
        window.saucer.call('mounted', []);
    }, 500);
});

function close() {
    window.saucer.call('close', []);
}

function minimize() {
    window.saucer.call('minimize', []);
}
</script>

<template>
    <nav>
        <div class="title-bar">
            <div class="title" ref="titleBar">
                <span>{{ title }}</span>
            </div>
            <div class="nav-controls">
                <Transition name="fade">
                    <RouterLink id="home" to="/" v-if="$route.path != '/'">
                        <IconBack></IconBack>
                    </RouterLink>
                </Transition>
            </div>
            <div class="controls">
                <Transition name="fade">
                    <RouterLink id="settings" to="/settings" v-if="$route.path != '/settings'">
                        <IconSettings></IconSettings>
                    </RouterLink>
                </Transition>
                <button id="minimize" @click="minimize">
                    <IconMinimize></IconMinimize>
                </button>
                <button id="close" @click="close">
                    <IconClose></IconClose>
                </button>
            </div>
        </div>
    </nav>
</template>
<style scoped>
nav {
    top: 0;
    left: 0;
    position: fixed;
    z-index: 60;
}

.title-bar {
    background-color: rgba(0, 0, 0, 0.5);
    backdrop-filter: blur(5px);
    text-align: center;
    line-height: 38px;
    background: linear-gradient(to top, rgba(0, 0, 0, 0.35) 0%, rgba(0, 0, 0, 0.5) 50%, rgba(20, 20, 20, 0.5) 55%, rgba(80, 80, 80, 0.5) 100%);
    border: 1px solid rgba(127, 127, 127, 0.1);
    border-top: 1px solid rgba(255, 255, 255, 0.6);
    
    box-shadow: 0px 5px 16px rgba(0, 0, 0, 0.5);
}

.title {
    width: 100vw;
    z-index: 9;
    text-shadow: 0px 2px 5px rgba(255, 255, 255, 0.5), 
        0px 2px 10px rgba(255, 255, 255, 0.5),
        0px 2px 20px rgba(255, 255, 255, 0.5);
}

.controls,
.nav-controls {
    position: absolute;
    right: 0;
    top: 0;
    height: 100%;
    display: flex;
}

.nav-controls {
    left: 0;
    width: 0;
    display: inline-block;
}

.controls>*,
.nav-controls>* {
    margin: 0;
    display: block;
    background-color: rgba(0, 0, 0, 0);
    border: none;
    padding: 0;
    color: #fff;
    width: 38px;
    line-height: 38px;
    transition: 0.1s ease;
    font-size: 24px;
}

#close:hover {
    background-color: rgb(226, 46, 22);
}

#close:active {
    background-color: rgb(153, 33, 17);
}

#minimize:hover {
    background-color: rgb(161, 22, 226);
}

#minimize:active {
    background-color: rgb(106, 14, 148);
}

#settings:hover {
    background-color: rgb(22, 151, 226);
}

#settings:active {
    background-color: rgb(18, 103, 153);
}

#home:hover {
    background-color: rgb(20, 187, 101);
}

#home:active {
    background-color: rgb(11, 114, 61);
}

.fade-enter-active,
.fade-leave-active {
    transition: 0.2s ease;
}

.fade-enter-from, 
.fade-leave-to {
    opacity: 0;
    transform: translateY(-100px);
}
</style>
