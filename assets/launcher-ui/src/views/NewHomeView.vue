<script setup lang="ts">
import { FontAwesomeIcon } from '@fortawesome/vue-fontawesome';
import { faArrowRight, faArrowUpFromBracket, faBan, faDownload, faGear, faGears, faPalette, faPlay } from '@fortawesome/free-solid-svg-icons';
import { computed, nextTick, onMounted, ref, useTemplateRef, watch, type VNodeRef } from 'vue';
import { iidx } from '@/modules/iidx';
import { sdvx } from '@/modules/sdvx';
import { gitadora } from '@/modules/gitadora';
import { ddr } from '@/modules/ddr';
import { launcher, VersionState } from "@/modules/launcher";
import type { GameClass } from '@/modules/gameClass';

const transMode = ref('down');
const games: GameClass[] = [ddr, gitadora, sdvx, iidx];
const selectedGame = ref((() => {
    const lastGame = localStorage.getItem('last-game');
    return lastGame ? parseInt(lastGame) : 0;
})());

watch(selectedGame, (index, oldIndex) => {
    localStorage.setItem('last-game', index.toString());
    transMode.value = index > oldIndex ? 'up' : 'down';
});

const selectedGameIndex = computed(() => {
    return games.length - selectedGame.value - 1;
});

onMounted(() => {
    (async () => {
        await launcher.loadConfig();
        await iidx.loadConfig();
        await sdvx.loadConfig();
        await iidx.updateMeta();
        await sdvx.updateMeta();
        await gitadora.updateMeta();
        await ddr.updateMeta();
    })();
});
</script>

<template>
    <main>
        <Transition :name="transMode" v-for="game in games">
            <div v-if="selectedGame == game.gameIndex" :class="['game-page', game.className]">
                <div class="gtdr-game-select"
                    v-if="game.className == 'gitadora' && game.installed && game.versionState.value == VersionState.Normal">
                    <div class="gf" @click="gitadora.startGf">
                        <div>GUITARFREAKS</div>
                    </div>
                    <div class="dm" @click="gitadora.startDm">
                        <div>DRUMMANIA</div>
                    </div>
                </div>
                <div class="iidx-chara" v-if="game.className == 'iidx'">
                </div>
            </div>

        </Transition>
        <div class="game-select">
            <Transition :name="transMode" v-for="game in games">
                <div v-if="selectedGame == game.gameIndex">
                    <div class="flex">
                        <div id="text">
                            {{ game.name }}
                        </div>
                        <div class="miscs" v-if="game.installed && game.versionState.value == VersionState.Normal">
                            <template v-if="game.className == 'iidx'">
                                <RouterLink to="/iidx/settings">
                                    <button class="crystal" title="打开更多设置">
                                        <div class="gloss"></div>
                                        <div class="shadow"></div>
                                        <div class="content">
                                            <FontAwesomeIcon :icon="faGears"></FontAwesomeIcon>
                                        </div>-
                                    </button>
                                </RouterLink>
                                <button class="crystal" title="打开自定义" :onclick="() => iidx.openCustomize()">
                                    <div class="gloss"></div>
                                    <div class="shadow"></div>
                                    <div class="content">
                                        <FontAwesomeIcon :icon="faPalette"></FontAwesomeIcon>
                                    </div>-
                                </button>
                            </template>
                            <button class="crystal" title="打开游戏设置" :onclick="() => game.settings()">
                                <div class="gloss"></div>
                                <div class="shadow"></div>
                                <div class="content">
                                    <FontAwesomeIcon :icon="faGear"></FontAwesomeIcon>
                                </div>-
                            </button>
                            <button class="crystal" title="打开更新器" :onclick="() => game.updater()">
                                <div class="gloss"></div>
                                <div class="shadow"></div>
                                <div class="content">
                                    <FontAwesomeIcon :icon="faDownload"></FontAwesomeIcon>
                                </div>-
                            </button>
                        </div>
                    </div>
                </div>
            </Transition>
        </div>
        <div :data-index="selectedGameIndex" class="game-icons">
            <div v-for="game in games"
                :class="['game', game.className, selectedGame == game.gameIndex ? 'selected' : '']"
                @click="() => selectedGame = game.gameIndex">
                <button disabled title="该游戏未安装" v-if="!game.installed">
                    <FontAwesomeIcon :icon="faDownload" size="2x" />
                </button>
                <button disabled title="请更新游戏" v-else-if="game.versionState.value == VersionState.Need2UpdateGame">
                    <FontAwesomeIcon :icon="faArrowUpFromBracket" size="2x" />
                </button>
                <button disabled title="启动器不支持该版本"
                    v-else-if="game.versionState.value == VersionState.Need2UpdateLauncher">
                    <FontAwesomeIcon :icon="faBan" size="2x" />
                </button>
                <button disabled v-else-if="game.className == 'gitadora'" title="请选择启动模式">
                    <FontAwesomeIcon :icon="faArrowRight" size="2x" />
                </button>
                <button v-else title="启动游戏" :onclick="() => games.find(g => g.gameIndex == selectedGame)?.start()">
                    <FontAwesomeIcon :icon="faPlay" size="2x" />
                </button>
            </div>
        </div>
    </main>
</template>
<style scoped lang="scss">
.iidx-chara {
    width: 100vw;
    height: 100vh;
    background-size: 85vw;
    background-position: bottom right;
    background-image: url(../assets/iidx_chara.png);
    background-repeat: no-repeat;
}

.gtdr-game-select {
    display: flex;
    height: 100%;
    width: calc(100vw - 200px);
    margin: auto;
    margin-left: 200px;
    justify-content: space-around;
    align-items: end;
}

.gtdr-game-select div {
    background-size: cover;
    background-position: top;
    background-repeat: no-repeat;
    width: 50%;
    height: 90%;
    display: flex;
    justify-content: center;
    align-items: center;
    filter: grayscale(1);
    cursor: pointer;
}

.gtdr-game-select div:hover {
    filter: grayscale(0);
}

.gtdr-game-select div div {
    margin-bottom: 2em;
    font-size: 38px;
    text-shadow: 0px 3px 0px black,
        0px 2px 5px black,
        0px 5px 10px black;
}

.gtdr-game-select>.dm {
    background-image: url(../assets/dm_chara.png);
}

.gtdr-game-select>.gf {
    background-image: url(../assets/gf_chara.png);
}

.up-enter-active,
.up-leave-active,
.up-enter-from,
.up-leave-to,
.down-enter-active,
.down-leave-active,
.down-enter-from,
.down-leave-to {
    transition: all 0.3s ease;
}

.up-enter-active,
.up-leave-active,
.down-enter-active,
.down-leave-active {
    position: absolute;
    top: 0;
}

.up-enter-from {
    opacity: 0;
    top: -64px;
}

.up-leave-to {
    opacity: 0;
    top: 64px;
}

.down-enter-from {
    opacity: 0;
    top: 64px;
}

.down-leave-to {
    opacity: 0;
    top: -64px;
}

main {
    height: 100vh;
    display: flex;
    align-items: stretch;
}

.game-page {
    width: 100vw;
    height: 100vh;
    background-size: cover;
    background-position: center;
}

.game-page.sdvx {
    background-image: url(@/assets/sdvx.jpg);
    background-position: center 38px;
    background-repeat: no-repeat;
}


.game-page.gitadora {
    background-image: url(@/assets/gitadora.jpg);
}

.game-page.ddr {
    background-image: url(@/assets/ddr.webp);
    background-position: bottom;
}

.game-page.iidx {
    background-image: url(@/assets/iidx.jpg);
}

.game-select {
    z-index: 1;
    background: linear-gradient(to top, rgba(0, 0, 0, 0.35) 0%, rgba(0, 0, 0, 0.5) 70%, rgba(20, 20, 20, 0.5) 71%, rgba(80, 80, 80, 0.5) 100%);
    background-color: rgba(0, 0, 0, 0.15);
    border-top: 1px solid rgba(255, 255, 255, 0.6);
    border-bottom: 1px solid rgba(127, 127, 127, 0.1);
    position: absolute;
    left: 0px;
    top: calc(50vh - 36px + 150px);
    height: 72px;
    width: 100vw;
    box-shadow: 0px 5px 16px rgba(0, 0, 0, 0.5);
    backdrop-filter: blur(5px);

}

.game-select::after {
    content: ' ';
    z-index: -1;
    position: absolute;
    left: 0px;
    top: 0px;
    width: 100%;
    height: 100%;
    background: url(../assets/aero.png);
    background-size: cover;
}

.game-select>div {
    width: 100%;
}

.game-select .flex {
    display: flex;
    align-items: center;
    justify-content: space-between;
}

.game-select #text {
    line-height: 72px;
    font-size: 24px;
    margin-left: 240px;
    text-shadow: 0px 2px 0px rgba(0, 0, 0, 0.1),
        0px 0px 5px rgba(255, 255, 255, 0.4),
        0px 0px 10px rgba(255, 255, 255, 0.5),
        0px 0px 20px rgba(255, 255, 255, 0.6);
}

.game-select .miscs {
    margin-right: 120px;
    text-align: right;
    display: flex;
    align-items: center;
}

.game-select .miscs>* {
    margin-left: 1em;
}

.crystal {
    content: ' ';
    outline: none;
    appearance: none;
    border: none;
    width: 48px;
    height: 48px;
    border-radius: 48px;
    position: relative;
    overflow: hidden;
    filter: grayscale(0.9);
    display: block;
    cursor: pointer;
}

.crystal::after {
    content: ' ';
    z-index: 0;
    position: absolute;
    top: -2.5%;
    left: -2.5%;
    width: 105%;
    height: 105%;
    border-radius: 48px;
    background-color: rgb(25, 220, 253);
    border-bottom: 5px solid rgba(255, 255, 255, 0.8);
    box-shadow:
        inset 0px 0px 12px rgba(0, 0, 0, 0.5),
        inset 0 32px 16px rgb(2, 35, 56);
    filter: blur(1.8px);
    transition: all 0.1s ease;
}

.crystal:hover::after {
    background-color: rgb(81, 229, 255);
    border-bottom: 5px solid rgba(255, 255, 255, 1);
    filter: blur(3px);
    box-shadow:
        inset 0px 0px 12px rgba(0, 0, 0, 0.5),
        inset 0 32px 16px rgb(0, 56, 91);
}

.crystal:active::after {
    background-color: rgb(25, 220, 253);
    border-bottom: 0px solid rgba(255, 255, 255, 0.8);
    box-shadow:
        inset 0px 0px 12px rgba(0, 0, 0, 10),
        inset 0 32px 16px rgb(2, 35, 56);
}

.crystal>.content {
    position: absolute;
    z-index: 1;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    line-height: 48px;
    font-size: 18px;
    text-shadow: 0 0 5px black, 0 0 5px black, 0 0 5px black;
}

.crystal>.gloss {
    position: absolute;
    z-index: 2;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    mix-blend-mode: screen;
    background: linear-gradient(to top, #212121 0%, #000 50%, #333333 52%, #c6c6c6 100%);
    border-radius: 48px;
    opacity: 1;
}

.crystal>.shadow {
    position: absolute;
    z-index: 3;
    top: -0.5%;
    left: -0.5%;
    width: 101%;
    height: 101%;
    border-radius: 48px;
    box-shadow:
        inset 0px 0px 3px rgba(0, 0, 0, 0.75),
        inset 0px 0px 3px rgba(0, 0, 0, 0.75),
        inset 0px 0px 3px rgba(0, 0, 0, 0.75);
}

.crystal:active>.shadow {
    box-shadow:
        inset 0px 0px 5px rgba(0, 0, 0, 0.75),
        inset 0px 0px 5px rgba(0, 0, 0, 0.75),
        inset 0px 0px 5px rgba(0, 0, 0, 0.75);
}

.game-icons {
    z-index: 2;
    display: flex;
    flex-direction: column;
    justify-content: flex-start;
    width: 48px;
    height: 200vh;
    position: absolute;
    left: 72px;

    transition: transform 0.2s ease;
    --index: attr(data-index type(<number>));
    transform: translateY(calc((50vh - 64px + 150px) - (var(--index) * 80px)));
}

.game.ddr {
    background-image: url(@/assets/icons/ddr.ico);
}

.game.sdvx {
    background-image: url(@/assets/icons/sdvx.ico);
}

.game.gitadora {
    background-image: url(@/assets/icons/gtdr.ico);
}

.game.iidx {
    background-image: url(@/assets/icons/iidx.ico);
}

.game-icons>.game {
    cursor: pointer;
    margin: 16px;
    border: 1px solid rgba(255, 255, 255, 0.1);
    border-top: 1px solid #fff;
    border-radius: 5px;
    width: 48px;
    height: 48px;
    overflow: hidden;
    transition: all 0.2s ease;
    width: 48px;
    height: 48px;
    display: block;
    background-size: cover;
    position: relative;
    box-shadow: 0px 16px 16px rgba(0, 0, 0, 0.5), 0px 8px 8px rgba(0, 0, 0, 0.5), 0px 4px 4px rgba(0, 0, 0, 0.5);
}

.game-icons>.game::after {
    content: ' ';
    position: absolute;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    background: linear-gradient(to top, rgba(0, 0, 0, 0.1) 0%, rgba(0, 0, 0, 0.2) 70%, rgba(20, 20, 20, 0.1) 71%, rgba(80, 80, 80, 0) 100%);
}

.game-icons>.game.selected {
    width: 96px;
    height: 96px;
}

.game-icons>.game.selected:hover {
    transform-origin: center;
    transform: translateY(-2px);
}


.game-icons>.game:not(.selected)>* {
    display: none;
}

svg.svg-inline--fa {
    margin-right: 0 !important;
}

.game-icons>.game>button:disabled {
    cursor: not-allowed;
}

.game-icons>.game>button {
    z-index: 1;
    appearance: none;
    outline: none;
    border: none;
    cursor: pointer;
    text-align: center;
    font-size: 32px;
    position: absolute;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    background-color: rgba(0, 0, 0, 0.2);
    color: #fff;
    opacity: 0;
    transition: opacity 0.3s ease;
}

.game-icons>.game.selected:hover>button {
    opacity: 1;
}
</style>
