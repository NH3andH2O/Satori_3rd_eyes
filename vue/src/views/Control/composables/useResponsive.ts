import { ref, computed, onMounted, onBeforeUnmount } from 'vue';
import { BREAKPOINTS } from '@/config';

/**
 * 響應式窗口尺寸管理
 */
export function useResponsive() {
	const width = ref(window.innerWidth);
	const height = ref(window.innerHeight);

	function handleResize() {
		width.value = window.innerWidth;
		height.value = window.innerHeight;
	}

	onMounted(() => {
		window.addEventListener('resize', handleResize);
	});

	onBeforeUnmount(() => {
		window.removeEventListener('resize', handleResize);
	});

	const isMobile = computed(() => width.value <= BREAKPOINTS.MOBILE);
	const isTablet = computed(() => width.value > BREAKPOINTS.MOBILE && width.value <= BREAKPOINTS.TABLET);
	const isDesktop = computed(() => width.value > BREAKPOINTS.TABLET);

	return {
		width,
		height,
		isMobile,
		isTablet,
		isDesktop,
	};
}
