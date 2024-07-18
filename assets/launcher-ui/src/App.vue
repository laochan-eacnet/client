<script setup lang="ts">
import { RouterView } from 'vue-router'
import TitleBar from './components/TitleBar.vue'
</script>

<template>
  <TitleBar title="Laochan Eacnet Launcher" />
  <RouterView />
</template>

<style scoped>
* {
  user-select: none;
}
</style>
