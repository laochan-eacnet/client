<script setup lang="ts">
import { onMounted, ref } from 'vue';
const version = ref('r0 - unknown branch')

onMounted(async () => {
    version.value = await window.laochan.version();
});

function num() {
    window.laochan.num();
}
</script>

<template>
    <div class="version" @click="num()">{{ version }}</div>
</template>

<style scoped>
.version {
    position: fixed;
    z-index: 30;
    left: 2em;
    bottom: 1em;
    color: #555;
  mix-blend-mode: screen;
}
</style>
