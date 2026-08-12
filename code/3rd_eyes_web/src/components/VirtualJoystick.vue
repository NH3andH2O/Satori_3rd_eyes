<script setup lang="ts">
import { onMounted, onBeforeUnmount, ref, watch } from 'vue';

// 注意：类型引入可选
// @ts-ignore
import nipplejs from 'nipplejs';

type Mode = 'dynamic' | 'static' | 'semi';

const props = defineProps<{
	mode?: Mode;
	size?: number;
	color?: string;
	// 固定位置，仅 static/semi 模式有效
	position?: { left?: string; top?: string; right?: string; bottom?: string };
	lockX?: boolean;
	lockY?: boolean;
	// 小幅抖动过滤，0~1，建议 0.08~0.2
	deadzone?: number;
	// move 事件节流（毫秒）
	throttle?: number;
	// 归一化转为 -100..100 的倍数（默认 100）
	scale?: number;
	// 是否将“上”视为正（很多机器人/车辆惯例）
	invertYPositive?: boolean;
}>();

const emits = defineEmits<{
	(e: 'start'): void;
	(e: 'end'): void;
	(
		e: 'move',
		payload: {
			// 原始（-1..1）：基于向量
			x: number;
			y: number;
			// 死区&缩放 (-scale..scale)，默认 -100..100
			vx: number;
			vy: number;
			// 其它信息
			angle?: number; // degree
			distance?: number; // px
			force?: number;
			direction?: string;
		},
	): void;
}>();

const zoneRef = ref<HTMLDivElement | null>(null);
let manager: any = null;
let lastEmit = 0;

const cfg = {
	mode: props.mode ?? 'dynamic',
	size: props.size ?? 120,
	color: props.color ?? '#3b82f6',
	position: props.position ?? { left: '50%', top: '50%' },
	lockX: !!props.lockX,
	lockY: !!props.lockY,
	deadzone: props.deadzone ?? 0.12,
	throttle: props.throttle ?? 60, // ms
	scale: props.scale ?? 100,
	invertYPositive: props.invertYPositive ?? true,
};

function applyDeadzoneAndScale(x: number, y: number) {
	// x,y 为 -1..1 的单位向量
	const mag = Math.hypot(x, y);
	if (mag < cfg.deadzone) return { vx: 0, vy: 0 };
	const scaled = (mag - cfg.deadzone) / (1 - cfg.deadzone);
	const nx = (x / mag) * scaled;
	const ny = (y / mag) * scaled;
	// 变换到 [-scale, scale]
	let vx = Math.round(nx * cfg.scale);
	let vy = Math.round(ny * cfg.scale);

	// 是否把“上”为正：浏览器坐标里上推通常是 y 负
	if (cfg.invertYPositive) vy = -vy;

	// 轴锁
	if (cfg.lockX) vx = 0;
	if (cfg.lockY) vy = 0;
	return { vx, vy };
}

function extractVector(data: any) {
	// nipplejs 的 data.vector 为单位向量（-1..1）
	let x = data?.vector?.x ?? 0;
	let y = data?.vector?.y ?? 0;

	// 轴锁
	if (cfg.lockX) x = 0;
	if (cfg.lockY) y = 0;

	// 若需要将上推视为正，原始 x,y 不变，这里只影响 vx/vy
	return { x, y };
}

function init() {
	if (!zoneRef.value) return;
	manager = nipplejs.create({
		zone: zoneRef.value,
		mode: cfg.mode,
		color: cfg.color,
		size: cfg.size,
		position: cfg.position, // static/semi 有效
		// 其它可选项：restOpacity, catchDistance, shape等
	});

	manager.on('start', () => emits('start'));

	manager.on('move', (_evt: any, data: any) => {
		const now = performance.now();
		if (now - lastEmit < cfg.throttle) return;
		lastEmit = now;

		const { x, y } = extractVector(data);
		const { vx, vy } = applyDeadzoneAndScale(x, y);
		emits('move', {
			x,
			y,
			vx,
			vy,
			angle: data?.angle?.degree,
			distance: data?.distance,
			force: data?.force,
			direction: data?.direction?.angle,
		});
	});

	manager.on('end', () => {
		emits('end');
		// 抬起时归零
		emits('move', { x: 0, y: 0, vx: 0, vy: 0, angle: 0, distance: 0, force: 0, direction: undefined });
	});
}

onMounted(() => {
	// SSR/NUXT 安全：仅在客户端初始化
	if (typeof window !== 'undefined') init();
});

onBeforeUnmount(() => {
	if (manager) {
		manager.destroy();
		manager = null;
	}
});

// 动态响应少量可热更的 props（颜色/尺寸）
watch(
	() => [props.color, props.size],
	() => {
		if (!manager) return;
		manager.destroy();
		manager = null;
		// 重新同步配置
		cfg.color = props.color ?? cfg.color;
		cfg.size = props.size ?? cfg.size;
		init();
	},
);
</script>

<template>
	<!-- 你可以给这个容器设定固定/自适应大小和背景网格等 -->
	<div ref="zoneRef" class="vj-zone" />
</template>

<style scoped>
.vj-zone {
	position: absolute;
	inset: 0; /* 等价 top/right/bottom/left: 0 */
	width: 100%;
	height: 100%;
	touch-action: none;
	user-select: none;
}
</style>
