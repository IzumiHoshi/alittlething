# 取消所有喜欢的twitter

1. 先上代码  

    ```js
    setInterval(function(){
    var divs = document.getElementsByTagName('div')
    var arr = Array.prototype.slice.call( divs)
    var hearts = arr.filter(x => x.getAttribute('data-testid') == 'unlike')
    hearts.forEach(h => h.click())
    window.scrollTo(0, document.body.scrollHeight ||document.documentElement.scrollHeight);
    },1000);
    ```

2. 进入到twiiter的like页面。按F12打开Console页面。将代码粘贴到页面中并运行。
3. 代码自动运行并自动翻页直到取消所有like。
