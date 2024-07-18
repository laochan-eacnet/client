<script setup lang="ts">
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
      </div>
      <div class="options">
        <div>启动</div>
        <div>Laochan 设置</div>
        <div>游戏设置</div>
        <div>更新器</div>
        <div>自定义选项</div>
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

<style scoped>
main {
  height: 100vh;
  display: flex;
  align-items: stretch;
  background-image: url(@/assets/moai-bg.jpg);
  background-size: cover;;
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
  filter: brightness(50%) opacity(90%);
  backdrop-filter: blur(10px);
  transition: 0.1s ease;
  z-index: -1;
}

.game>.text {
  text-align: center;
  margin: auto;
  margin-top: calc(40vh);
  font-size: 24px;
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  font-weight: 100;
  transition: 0.1s ease;
}

.game:not(.disable):hover>.text  {
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

.game>.options>div {
  padding: 1em;
  text-align: center;
}

.game>.options>div:first-child {
  font-size: 24px;
}

.game>.options>div:hover {
  background-color: #fff;
  color: #000;
}
</style>