<template>
	<transition name="el-fade-in">
		<div v-if="visible">
			<el-tooltip :content="t('back')" placement="left" :effect="isDark ? 'dark' : 'light'">
				<div
					class="backtop-like"
					:style="{
						right: `${right}px`,
						bottom: `${bottom}px`,
						zIndex: zIndex,
					}"
					@click="handleClick"
					role="button"
					:aria-label="t('back')"
					tabindex="0"
					@keydown.enter.prevent="handleClick"
					@keydown.space.prevent="handleClick"
				>
					<slot>
						<div class="btn-content">
							<el-icon><ArrowLeft /></el-icon>
						</div>
					</slot>
				</div>
			</el-tooltip>
		</div>
	</transition>
</template>

<script setup lang="ts">
import { ref, onMounted, onBeforeUnmount } from 'vue';
import { useRouter, type RouteLocationRaw } from 'vue-router';
import { useDark } from '@vueuse/core';
import { ArrowLeft } from '@element-plus/icons-vue';
import { useI18n } from 'vue-i18n';

const { t } = useI18n();

const isDark = useDark();

const props = defineProps<{
	visibilityHeight?: number;
	right?: number;
	bottom?: number;
	zIndex?: number;
	returnTo?: string | RouteLocationRaw | (() => string | RouteLocationRaw);
	fallbackPath?: string;
	target?: string | Element;
}>();

const { visibilityHeight = 300, right = 40, bottom = 40, zIndex = 1000, fallbackPath = '/', target = '' } = props;

const router = useRouter();
const visible = ref(false);
let containerEl: Element | Window = window;

function isAbsoluteUrl(url: string) {
	return /^https?:\/\//i.test(url);
}
function getReturnTarget(): string | RouteLocationRaw {
	if (typeof props.returnTo === 'function') return props.returnTo();
	if (props.returnTo) return props.returnTo;
	return fallbackPath;
}

function handleClick() {
	const target = getReturnTarget();

	if (typeof target === 'string') {
		if (isAbsoluteUrl(target)) {
			window.location.assign(target);
		} else {
			router.push(target);
		}
		return;
	}

	router.push(target);
}

function getScrollTop(el: Element | Window | null) {
	if (!el || el === window) {
		return window.pageYOffset || document.documentElement.scrollTop || document.body.scrollTop || 0;
	}
	return (el as Element & { scrollTop?: number }).scrollTop || 0;
}
function updateVisibility() {
	visible.value = getScrollTop(containerEl) >= visibilityHeight;
}
function resolveTarget(): Element | Window {
	if (!target) return window;
	if (typeof target === 'string') return document.querySelector(target) || window;
	return target;
}
function onScroll() {
	updateVisibility();
}

onMounted(() => {
	containerEl = resolveTarget();
	containerEl.addEventListener('scroll' as any, onScroll, { passive: true } as any);
	updateVisibility();
});
onBeforeUnmount(() => {
	(containerEl as any)?.removeEventListener('scroll', onScroll);
});
</script>

<style scoped>
.backtop-like {
	position: fixed;
	display: inline-flex;
	align-items: center;
	justify-content: center;
	width: 48px;
	height: 48px;
	background: var(--el-color-primary);
	color: #fff;
	border-radius: 50%;
	cursor: pointer;
	box-shadow: 0 2px 12px rgba(0, 0, 0, 0.2);
	transition:
		transform 0.2s,
		box-shadow 0.2s,
		opacity 0.2s;
	user-select: none;
}
.backtop-like:hover {
	transform: translateY(-2px);
	box-shadow: 0 6px 18px rgba(0, 0, 0, 0.25);
}
.btn-content {
	display: inline-flex;
	align-items: center;
	gap: 6px;
	font-size: 20px;
	line-height: 1;
	padding: 0 10px;
}
</style>
