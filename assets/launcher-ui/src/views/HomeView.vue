<script setup lang="ts">
import { iidx } from '@/modules/iidx';
</script>

<template>
  <main>
    <div></div>
    <div class="game sdvx disable">
      <div class="background"></div>
      <div class="text">
        SOUND VOLTEX<br>
        EXCEED GEAR
        <div class="tip"><small>COMING S∞N</small></div>
      </div>
    </div>
    <div class="game iidx">
      <div class="background"></div>
      <div class="text">
        beatmania IIDX<br>
        INFINITAS
        <div v-if="!iidx.installed()" class="tip">
          未安装
        </div>
      </div>
      <div v-if="iidx.installed()" class="options">
        <div class="opt" @click="iidx.start">启动</div>
        <RouterLink class="opt" to="/iidx/settings">额外设置</RouterLink>
        <div class="opt" @click="iidx.settings">游戏设置</div>
        <div class="opt" @click="iidx.updater">更新器</div>
        <div class="opt" @click="iidx.openCustomize">自定义选项</div>
      </div>

    </div>
    <div class="game gitadora disable">
      <div class="background"></div>
      <div class="text">
        GITDORA
        <div class="tip"><small>COMING S∞N</small></div>
      </div>
    </div>
    <div></div>
  </main>
</template>

<style scoped lang="scss">
main {
  height: 100vh;
  display: flex;
  align-items: stretch;
  background-color: #000;
  background-image: url(@/assets/moai-bg.jpg);
  background-size: cover;
}

main>div {
  height: 100%;
  width: 100%;
  margin-left: -100px;
}

.game {
  transition: 0.5s ease;
  position: relative;
  clip-path: polygon(100px 0%, 100% 0%, calc(100% - 100px) 100%, 0% 100%);
}

.game>.background {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  background-position: center;
  backdrop-filter: brightness(50%);
  transition: 0.1s ease;
  z-index: -1;
}

.game>.text {
  text-align: center;
  margin: auto;
  margin-top: 250px;
  font-size: 24px;
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  font-weight: 100;
  transition: 0.1s ease;
}

.game:not(.disable):hover>.text {
  font-weight: normal;
  text-shadow: 0 0 10px black;
}

.game:not(.disable):hover {
  width: 250%;
}

.game:not(.disable):hover>.background {
  filter: brightness(100%);
}

.game.disable:hover {
  width: 120%;
}

.game>.text>.tip {
  opacity: 0%;
  transition: 0.1s ease;
  padding: 1em;
}

.game:hover>.text>.tip {
  opacity: 100%;
}

.iidx>.background {
  background-image: url(@/assets/iidx.jpg);
}

@property --tag-depth {
  syntax: "auto";
  inherits: true;
  initial-value: 0px;
}

.game>.options {
  cursor: pointer;
  transition: 0.1s ease;
  margin-top: 2em;
  font-size: 16px;
  opacity: 0%;
}

.game:hover>.options {
  opacity: 100%;
}

.game>.options>.opt {
  padding: 1em;
  text-align: center;
  display: block;
  color: #fff;
}

@for $i from 1 through 10 {
  .game>.options>.opt:nth-child(#{$i}) {
    margin-right: calc($i * 9px);
  }
}

.game>.options>.opt:first-child {
  font-size: 24px;
}

.game>.options>.opt:hover {
  background-color: #fff;
  color: #000;
}
</style>
