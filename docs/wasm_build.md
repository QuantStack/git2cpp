# WebAssembly build

The in-browser WebAssembly build of `git2cpp` is intended to behave as similar as possible to other
builds but there are some differences when remotely accessing remote servers, in particular with
blocking requests and the requirement for a CORS proxy server.


## Blocking requests

Remote `git2cpp` requests in non-WebAssembly builds are progressive and feedback is provided as the
data is streamed back from the remote. But in WebAssembly builds such remote requests are blocking
and no feedback can be provided until the entire response is received back from the remote.
This can be a long time to wait without feedback, and if there is an error such that a response is
not received it could block forever, leaving the in-browser terminal unusable.

Hence the WebAssembly build limits http(s) requests with a timeout that defaults to 10 seconds.
This timeout can be increased using the [`GIT_HTTP_TIMEOUT` environment variable](git_http_timeout).

In addition, when a `git2cpp` response is received that is larger than 10 MB, a prompt is presented
to the user to confirm whether to proceed to unpack the response or not. Note that the size of the
response may be smaller or larger than the size of the directory structure it unpacks to.


(cors_proxy)=
## CORS proxy server

The fetching of resources in a browser from one domain to another is often limited by a browser
security feature called
[Cross-Origin Resource Sharing](https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/CORS)
(CORS). For this to be allowed the target domain must indicate it is happy to accept cross-origin
requests by adding certain headers to `https` responses. Most git servers such as `github.com` do
not add these headers, so a `git2cpp clone` from `github.com` will fail with a CORS error if run
from within a browser whereas there is no such limitation if run from a terminal of a real computer.

The solution to this problem is to use a separate CORS proxy server. The `git2cpp` remote request is
sent to this intermediate server which send the request to the target server and as this request is
coming from outside a browser it is not subject to CORS restrictions. The proxy receives the
response from the target server and adds the required CORS headers before returning it to `git2cpp`.

Various public CORS proxy servers are available for use or you serve your own. It can be useful to
serve your own on `localhost` when experimenting to confirm that everything works as expected
before moving on to a more complex solution. Be aware that the CORS proxy server is able to read the
content of your request so be careful if you are using authentication tokens.

The [`GIT_CORS_PROXY` environment variable](git_cors_proxy) is used to specify how the target URL is
encoded in the CORS proxy URL.

### Example running a local CORS proxy server

If you are running a local [cockle](https://github.com/jupyterlite/cockle) or
[JupyterLite terminal](https://github.com/jupyterlite/terminal) deployment you can also run a local
CORS proxy such as [CORS Anywhere](https://github.com/Rob--W/cors-anywhere) to test it out.
In a separate terminal on your host machine on which you have `nodejs` available, `cd` to a new
clean directory, and download and run the CORS proxy server using:

```js
npm install cors-anywhere
HOST=localhost PORT=8881 node node_modules/cors-anywhere/server.js
```

This will start the CORS proxy server listening on `http://localhost:8881/`. To use this in your
local `cockle` or `JupyterLite terminal` deployment in your browser set the `CORS_PROXY_URL` to be
```bash
export GIT_CORS_PROXY=http://localhost:8881/
```
and then try a `git2cpp clone` using something like:
```bash
git2cpp clone https://github.com/some-organisation/some-repository
```

### Example using a public CORS proxy server

There is a public instance of [CORS Anywhere](https://github.com/Rob--W/cors-anywhere) available at
`https://cors-anywhere.herokuapp.com/`. This can be used for demonstration purposes but it requires
an explicit opt-in and your access will be time-limited. To request temporary access go to
`https://cors-anywhere.herokuapp.com/` and follow the instructions there.

Once you have access, you can try this out in one of the public deployments such as those at
[https://jupyterlite.github.io/cockle](https://jupyterlite.github.io/cockle) or
[https://jupyterlite.github.io/terminal](https://jupyterlite.github.io/terminal).

Set the `CORS_PROXY_URL` to be
```bash
export GIT_CORS_PROXY=https://cors-anywhere.herokuapp.com/
```
and then try a `git2cpp clone` using something like:
```bash
git2cpp clone https://github.com/some-organisation/some-repository
```
