// The entry file of your WebAssembly module.

let nextBlock: i32 = 0;

const IS_FREE_FLAG: i32 = 0x1;
const HEADER_FOOTER_SIZE: i32 = 8;
const HEAP_START: i32 = 4;
const PAGE_SIZE: i32 = 64 * 1024;
const MIN_BLOCK_SIZE: i32 = 16;

function initBlock(at: i32, size: i32, isFree: boolean): void {
  let headerValue = size | (isFree ? IS_FREE_FLAG : 0);
  store<i32>(at, headerValue);
  store<i32>(at - 4, headerValue);

  if (isFree) {
    store<i32>(at + 4, 0);
    store<i32>(at + 8, 0);
  }
}

function isFree(at: i32): boolean {
  return (load<i32>(at) & 0x1) == 0x1;
}

function getSize(at: i32): i32 {
  return load<i32>(at) & ~0x1;
}

function canUseBlock(at: i32, contentSize: i32): boolean {
  const data = load<i32>(at);
  const size = data & ~IS_FREE_FLAG;

  return size == data && size >= contentSize + HEADER_FOOTER_SIZE;
}

function alignBlockSize(size: i32): i32 {
  return (size + MIN_BLOCK_SIZE - 1) & ~(MIN_BLOCK_SIZE - 1);
}

function initHeap():void {
  nextBlock = HEAP_START;
  const blockSize = memory.size() * PAGE_SIZE - HEAP_START;
  initBlock(nextBlock, blockSize, true);
}

function removeFreeBlock(at: i32): void {

}

export function malloc(size: i32): i32 {
  if (nextBlock == 0) {
    initHeap();
  }

  size = alignBlockSize(size);

  let current = nextBlock;

  while (current !== 0 && !canUseBlock(current, size)) {
    assert(isFree(current));
    current = load<i32>(current, 4);
  }

  if (current !== 0) {
    removeFreeBlock(current);
  }
  
  return nextBlock;
}
