import axios from 'axios';

export async function fetchWasm(uri: string) {
  const { data: bufferSource } = await axios({
    url: uri,
    method: 'get',
    responseType: 'arraybuffer',
  });

  return bufferSource;
}
