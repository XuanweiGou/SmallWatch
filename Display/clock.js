class LuxuryClock {
    constructor(canvas) {
        this.canvas = canvas;
        this.ctx = canvas.getContext('2d');
        this.radius = 120;
        this.centerX = this.radius;
        this.centerY = this.radius;
        this.frameCount = 0;
        this.FRAMES_PER_STEP = 30;
        this.lastTime = Date.now();
        this.lastStepTime = 0;
        this.init();
    }

    init() {
        // 初始化完成后自动开始
        this.start();
    }

    drawFace() {
        // 清除画布
        this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
        
        // 绘制主表盘 - 玫瑰金底色
        this.ctx.beginPath();
        this.ctx.arc(this.centerX, this.centerY, this.radius, 0, 2 * Math.PI);
        
        // 创建金属质感渐变
        const baseGradient = this.ctx.createRadialGradient(
            this.centerX - this.radius * 0.3, this.centerY - this.radius * 0.3, 0,
            this.centerX, this.centerY, this.radius
        );
        baseGradient.addColorStop(0, '#f7e8e3');  // 更浅的玫瑰色
        baseGradient.addColorStop(0.5, '#f2dcd4');  // 中间色
        baseGradient.addColorStop(1, '#e8cec7');  // 深色
        this.ctx.fillStyle = baseGradient;
        this.ctx.fill();

        // 绘制静态波纹效果
        const totalRipples = 4;
        
        // 定义波纹的间距分布函数
        const getSpacing = (progress) => {
            // 调整间距分布
            if (progress < 0.25) {
                return this.radius * (0.1 - progress * 0.2);
            }
            const baseSpacing = this.radius * 0.06;
            const variableSpacing = this.radius * 0.04 * Math.pow(progress, 0.7);
            return baseSpacing + variableSpacing;
        };

        // 定义波纹的强度分布函数
        const getIntensity = (progress) => {
            if (progress < 0.25) {
                return 0.04 * (1 - progress * 2);
            }
            const baseIntensity = 0.1;
            return baseIntensity * Math.exp(-2.5 * progress);
        };

        // 绘制波纹
        for(let i = 0; i < totalRipples; i++) {
            const progress = i / totalRipples;
            
            // 调整起始半径，从更外圈开始
            let currentRadius = this.radius * (0.45 + Math.pow(progress, 0.6) * 0.5);
            
            // 添加轻微的不规则性
            const variation = Math.sin(progress * Math.PI * 4) * this.radius * 0.01;
            currentRadius += variation;
            
            const intensity = getIntensity(progress);
            const spacing = getSpacing(progress);
            
            // 绘制主波纹
            this.ctx.beginPath();
            this.ctx.arc(this.centerX, this.centerY, currentRadius, 0, 2 * Math.PI);
            this.ctx.strokeStyle = `rgba(0, 0, 0, ${intensity})`;
            this.ctx.lineWidth = spacing * (1 + Math.sin(progress * Math.PI) * 0.2);
            this.ctx.stroke();

            // 添加亮边效果
            this.ctx.beginPath();
            this.ctx.arc(this.centerX, this.centerY, currentRadius - spacing * 0.3, 0, 2 * Math.PI);
            this.ctx.strokeStyle = `rgba(255, 255, 255, ${intensity * 0.3})`;
            this.ctx.lineWidth = spacing * 0.4;
            this.ctx.stroke();
        }

        // 优化中心过渡
        const centerGradient = this.ctx.createRadialGradient(
            this.centerX, this.centerY, 0,
            this.centerX, this.centerY, this.radius * 0.45
        );
        centerGradient.addColorStop(0, '#ffffff');
        centerGradient.addColorStop(0.3, 'rgba(255,255,255,0.95)');
        centerGradient.addColorStop(0.6, 'rgba(255,255,255,0.7)');
        centerGradient.addColorStop(0.8, 'rgba(255,255,255,0.3)');
        centerGradient.addColorStop(1, 'rgba(255,255,255,0)');
        
        this.ctx.beginPath();
        this.ctx.arc(this.centerX, this.centerY, this.radius * 0.45, 0, 2 * Math.PI);
        this.ctx.fillStyle = centerGradient;
        this.ctx.fill();

        // 添加同心圆纹理
        for(let i = 1; i <= 5; i++) {
            const circleRadius = this.radius * (0.2 + i * 0.15);
            this.ctx.beginPath();
            this.ctx.arc(this.centerX, this.centerY, circleRadius, 0, 2 * Math.PI);
            this.ctx.strokeStyle = `rgba(0, 0, 0, 0.02)`;
            this.ctx.lineWidth = 0.3;
            this.ctx.stroke();
        }

        // 修改时标为更大的圆润三角形
        for(let i = 0; i < 12; i++) {
            const angle = (i * Math.PI / 6) - Math.PI / 2;
            const innerRadius = this.radius * 0.75;  // 保持内径不变
            const outerRadius = this.radius * 0.95;  // 保持外径不变
            
            // 添加回 tip 的定义
            const tip = {
                x: this.centerX + Math.cos(angle) * innerRadius,
                y: this.centerY + Math.sin(angle) * innerRadius
            };
            
            // 增大顶角宽度（1.3倍）
            const base1 = {
                x: this.centerX + Math.cos(angle - 0.026) * outerRadius,
                y: this.centerY + Math.sin(angle - 0.026) * outerRadius
            };
            const base2 = {
                x: this.centerX + Math.cos(angle + 0.026) * outerRadius,
                y: this.centerY + Math.sin(angle + 0.026) * outerRadius
            };

            // 使用更平滑的贝塞尔曲线
            this.ctx.beginPath();
            this.ctx.moveTo(tip.x, tip.y);
            
            // 调整控制点位置使曲线更流畅
            const controlPoint1 = {
                x: this.centerX + Math.cos(angle - 0.01) * (innerRadius + (outerRadius - innerRadius) * 0.3),
                y: this.centerY + Math.sin(angle - 0.01) * (innerRadius + (outerRadius - innerRadius) * 0.3)
            };
            const controlPoint2 = {
                x: this.centerX + Math.cos(angle + 0.01) * (innerRadius + (outerRadius - innerRadius) * 0.3),
                y: this.centerY + Math.sin(angle + 0.01) * (innerRadius + (outerRadius - innerRadius) * 0.3)
            };

            this.ctx.bezierCurveTo(
                controlPoint1.x, controlPoint1.y,
                base1.x, base1.y,
                base1.x, base1.y
            );
            this.ctx.bezierCurveTo(
                base1.x, base1.y,
                base2.x, base2.y,
                base2.x, base2.y
            );
            this.ctx.bezierCurveTo(
                base2.x, base2.y,
                controlPoint2.x, controlPoint2.y,
                tip.x, tip.y
            );

            // 创建金属质感渐变
            const gradient = this.ctx.createLinearGradient(
                this.centerX + Math.cos(angle - 0.02) * innerRadius,
                this.centerY + Math.sin(angle - 0.02) * innerRadius,
                this.centerX + Math.cos(angle + 0.02) * innerRadius,
                this.centerY + Math.sin(angle + 0.02) * innerRadius
            );
            
            // 使用更细腻的金属渐变
            gradient.addColorStop(0, '#666666');    // 暗部
            gradient.addColorStop(0.3, '#cccccc');  // 降低高光强度，从e8e8e8改为cccccc
            gradient.addColorStop(0.5, '#eeeeee');  // 降低最亮部分，从ffffff改为eeeeee
            gradient.addColorStop(0.7, '#cccccc');  // 降低高光强度
            gradient.addColorStop(1, '#666666');    // 暗部
            
            this.ctx.fillStyle = gradient;
            this.ctx.fill();

            // 添加边缘高光
            this.ctx.strokeStyle = 'rgba(255, 255, 255, 0.2)';  // 从0.3降低到0.2
            this.ctx.lineWidth = 0.5;
            this.ctx.stroke();

            // 添加阴影效果
            this.ctx.shadowColor = 'rgba(0, 0, 0, 0.2)';
            this.ctx.shadowBlur = 2;
            this.ctx.shadowOffsetX = 1;
            this.ctx.shadowOffsetY = 1;
        }

        // 更新品牌名称样式
        this.ctx.font = 'italic 17px "Zapfino", "Edwardian Script ITC", "Kunstler Script", cursive';
        this.ctx.fillStyle = '#b76e5d';
        this.ctx.textAlign = 'center';
        // 添加装饰性笔画
        const brandName = 'YongqiGou';
        const x = this.centerX;
        const y = this.centerY - 30;
        
        // 添加装饰性底纹
        this.ctx.strokeStyle = 'rgba(51, 51, 51, 0.2)';
        this.ctx.lineWidth = 0.5;
        this.ctx.beginPath();
        this.ctx.moveTo(x - 40, y);
        this.ctx.bezierCurveTo(x - 20, y - 5, x + 20, y - 5, x + 40, y);
        this.ctx.stroke();
        
        // 绘制主文本
        this.ctx.fillText(brandName, x, y);
        
        // 添加装饰性顶纹
        this.ctx.beginPath();
        this.ctx.moveTo(x - 35, y - 2);
        this.ctx.bezierCurveTo(x - 15, y - 7, x + 15, y - 7, x + 35, y - 2);
        this.ctx.stroke();
    }

    drawDateWindow() {
        const centerX = this.centerX;
        const centerY = this.centerY + this.radius * 0.35 + 20;
        
        // 整体框的尺寸放大1.1倍
        const totalWidth = this.radius * 0.3 * 1.1;
        const boxHeight = this.radius * 0.12 * 1.1;
        
        // 计算Dec和01的宽度
        const decWidth = totalWidth * 0.67;
        const dateWidth = totalWidth * 0.33;
        const spacing = 4;
        
        // 计算两个框的位置，DEC框向左移动1像素
        const decX = centerX - (dateWidth + spacing) / 2 - 1;  // 向左移动1像素
        const dateX = centerX + (decWidth + spacing) / 2 - 1;  // 保持不变
        
        // 获取当前日期
        const now = new Date();
        const month = now.toLocaleString('en-US', { month: 'short' }).toUpperCase();
        const date = now.getDate().toString().padStart(2, '0');
        
        // 字体也相应放大1.1倍
        const monthFont = '12px "Times New Roman"';  // 从11px改为12px
        const dateFont = '11px "Times New Roman"';   // 从10px改为11px
        
        // 绘制Dec框
        this.drawDateBox(
            decX,
            centerY,
            decWidth,
            boxHeight,
            month,
            monthFont,
            4
        );

        // 绘制日期框
        this.drawDateBox(
            dateX,
            centerY,
            dateWidth,
            boxHeight,
            date,
            dateFont,
            4
        );
    }

    drawDateBox(x, y, width, height, text, font, padding) {
        this.ctx.save();
        
        // 添加阴影效果
        this.ctx.shadowColor = 'rgba(0, 0, 0, 0.1)';
        this.ctx.shadowBlur = 2;
        this.ctx.shadowOffsetX = 1;
        this.ctx.shadowOffsetY = 1;
        
        // 绘制白色背景
        this.ctx.beginPath();
        this.ctx.rect(x - width/2, y - height/2, width, height);
        this.ctx.fillStyle = '#fff';
        this.ctx.fill();
        
        // 绘制金属边框
        this.ctx.strokeStyle = '#d4b5ac';
        this.ctx.lineWidth = 1;
        this.ctx.stroke();
        
        // 添加金属光泽
        const gradient = this.ctx.createLinearGradient(
            x - width/2, y - height/2,
            x + width/2, y + height/2
        );
        gradient.addColorStop(0, 'rgba(255,255,255,0.5)');
        gradient.addColorStop(0.5, 'rgba(255,255,255,0)');
        gradient.addColorStop(1, 'rgba(255,255,255,0.5)');
        
        this.ctx.strokeStyle = gradient;
        this.ctx.lineWidth = 0.5;
        this.ctx.stroke();
        
        // 绘制文本 - 修改对齐方式
        this.ctx.font = font;
        this.ctx.fillStyle = '#d4b5ac';
        this.ctx.textAlign = 'center';
        this.ctx.textBaseline = 'middle';
        
        // 根据文本内容调整垂直位置
        let finalY;
        if (text.length > 2) {
            finalY = y + 1;  // DEC 保持向下移动1像素
        } else {
            finalY = y + 0.5;  // 01 向下移动0.5像素
        }
        
        this.ctx.fillText(text, x, finalY);
        
        this.ctx.restore();
    }

    drawHands(hours, minutes, seconds, milliseconds) {
        const now = Date.now();
        const stepDuration = 30; // 进一步减少步进持续时间
        
        // 计算机械步进效果
        let mechanicalSecondAngle;
        if (now - this.lastStepTime > 1000) {
            this.lastStepTime = now - (now % 1000);
        }
        
        const stepProgress = (now - this.lastStepTime) / stepDuration;
        if (stepProgress < 1) {
            // 使用线性动画，完全去除回弹
            const currentSecond = Math.floor(seconds);
            const currentAngle = currentSecond * (Math.PI / 30);
            const nextAngle = (currentSecond + 1) * (Math.PI / 30);
            mechanicalSecondAngle = currentAngle;  // 直接使用当前角度，不插值
        } else {
            mechanicalSecondAngle = seconds * Math.PI / 30;
        }
        mechanicalSecondAngle -= Math.PI / 2;

        // 调整阴影效果
        this.ctx.save();
        this.ctx.shadowColor = 'rgba(0, 0, 0, 0.15)';
        this.ctx.shadowBlur = 8;  // 减小模糊半径
        this.ctx.shadowOffsetX = 2;
        this.ctx.shadowOffsetY = 2;

        // 加深指针阴影
        this.ctx.save();
        this.ctx.shadowColor = 'rgba(0, 0, 0, 0.15)';  // 增加阴影不透明度
        this.ctx.shadowBlur = 15;
        this.ctx.shadowOffsetX = 3;
        this.ctx.shadowOffsetY = 3;

        const hoursAngle = (hours % 12 + minutes / 60) * Math.PI / 6 - Math.PI / 2;
        const minutesAngle = (minutes + seconds / 60) * Math.PI / 30 - Math.PI / 2;

        // 绘制阴影层
        this.drawHandShadow(hoursAngle, this.radius * 0.45, 6);      // 时针长度从0.4改为0.45
        this.drawHandShadow(minutesAngle, this.radius * 0.65, 4);    // 分针长度从0.55改为0.65
        this.drawHandShadow(mechanicalSecondAngle, this.radius * 0.8, 2);  // 秒针长度保持不变

        this.ctx.restore();

        // 绘制实际指针
        this.drawMetallicHand(hoursAngle, this.radius * 0.45, 6);    // 时针长度从0.4改为0.45
        this.drawMetallicHand(minutesAngle, this.radius * 0.65, 4);  // 分针长度从0.55改为0.65
        this.drawMetallicSecondHand(mechanicalSecondAngle, this.radius * 0.8); // 秒针长度保持不变

        this.drawHandsCenter();
    }

    drawHandShadow(angle, length, width) {
        this.ctx.save();
        this.ctx.translate(this.centerX, this.centerY);
        this.ctx.rotate(angle);
        
        // 绘制半透明阴影
        this.ctx.beginPath();
        this.ctx.moveTo(-width/2, 0);
        this.ctx.lineTo(-width/4, -length * 0.2);
        this.ctx.lineTo(0, -length);
        this.ctx.lineTo(width/4, -length * 0.2);
        this.ctx.lineTo(width/2, 0);
        this.ctx.closePath();
        
        this.ctx.fillStyle = 'rgba(0, 0, 0, 0.08)';  // 调整阴影透明度
        this.ctx.fill();
        
        this.ctx.restore();
    }

    drawMetallicHand(angle, length, width) {
        this.ctx.save();
        
        // 在绘制前设置抗锯齿
        this.ctx.lineCap = 'round';
        this.ctx.lineJoin = 'round';
        
        this.ctx.translate(this.centerX, this.centerY);
        this.ctx.rotate(angle);

        // 使用更高的分辨率绘制
        const scale = 2;
        this.ctx.scale(scale, scale);
        width = width / scale;
        length = length / scale;

        // 使用更平滑的贝塞尔曲线
        this.ctx.beginPath();
        this.ctx.moveTo(-width/2, 0);
        
        // 使用三次贝塞尔曲线创建更平滑的边缘
        this.ctx.bezierCurveTo(
            -width/2, -length * 0.2,
            -width/4, -length * 0.6,
            0, -length
        );
        this.ctx.bezierCurveTo(
            width/4, -length * 0.6,
            width/2, -length * 0.2,
            width/2, 0
        );
        this.ctx.closePath();

        // 创建更平滑的渐变
        const gradient = this.ctx.createLinearGradient(-width/2, 0, width/2, 0);
        gradient.addColorStop(0, '#666666');
        gradient.addColorStop(0.2, '#888888');
        gradient.addColorStop(0.35, '#aaaaaa');
        gradient.addColorStop(0.5, '#cccccc');
        gradient.addColorStop(0.65, '#aaaaaa');
        gradient.addColorStop(0.8, '#888888');
        gradient.addColorStop(1, '#666666');

        this.ctx.fillStyle = gradient;
        this.ctx.fill();

        // 添加更柔和的高光
        this.ctx.beginPath();
        this.ctx.moveTo(-width/4, -length * 0.2);
        this.ctx.quadraticCurveTo(0, -length * 0.6, 0, -length);
        this.ctx.strokeStyle = 'rgba(255,255,255,0.4)';
        this.ctx.lineWidth = 0.3;
        this.ctx.stroke();

        this.ctx.restore();
    }

    drawMetallicSecondHand(angle, length) {
        this.ctx.save();
        this.ctx.translate(this.centerX, this.centerY);
        this.ctx.rotate(angle);

        const width = 2; // 稍微减小秒针宽度

        // 创建更细腻的金属渐变
        const gradient = this.ctx.createLinearGradient(-width/2, 0, width/2, 0);
        gradient.addColorStop(0, '#444444');
        gradient.addColorStop(0.2, '#999999');
        gradient.addColorStop(0.3, '#f0f0f0');
        gradient.addColorStop(0.4, '#ffffff');
        gradient.addColorStop(0.5, '#f0f0f0');
        gradient.addColorStop(0.6, '#999999');
        gradient.addColorStop(0.8, '#666666');
        gradient.addColorStop(1, '#444444');

        // 使用贝塞尔曲线绘制更圆润的秒针
        this.ctx.beginPath();
        this.ctx.moveTo(-width/2, length * 0.2);
        this.ctx.bezierCurveTo(
            -width/2, length * 0.1,
            -width/4, 0,
            0, -length
        );
        this.ctx.bezierCurveTo(
            width/4, 0,
            width/2, length * 0.1,
            width/2, length * 0.2
        );
        this.ctx.closePath();

        this.ctx.fillStyle = gradient;
        this.ctx.fill();

        // 添加多层高光
        // 主高光
        this.ctx.beginPath();
        this.ctx.moveTo(-width/4, 0);
        this.ctx.quadraticCurveTo(0, -length * 0.6, 0, -length);
        this.ctx.strokeStyle = 'rgba(255,255,255,0.7)';
        this.ctx.lineWidth = 0.5;
        this.ctx.stroke();

        // 次高光
        this.ctx.beginPath();
        this.ctx.moveTo(-width/6, -length * 0.2);
        this.ctx.quadraticCurveTo(0, -length * 0.7, 0, -length);
        this.ctx.strokeStyle = 'rgba(255,255,255,0.4)';
        this.ctx.lineWidth = 0.3;
        this.ctx.stroke();

        // 装饰圆点也使用更细腻的渐变
        this.ctx.beginPath();
        this.ctx.arc(0, -length * 0.8, 2, 0, 2 * Math.PI);
        const circleGradient = this.ctx.createRadialGradient(
            -0.5, -length * 0.8 - 0.5, 0,
            0, -length * 0.8, 2
        );
        circleGradient.addColorStop(0, '#ffffff');
        circleGradient.addColorStop(0.3, '#cccccc');
        circleGradient.addColorStop(0.6, '#999999');
        circleGradient.addColorStop(1, '#666666');
        this.ctx.fillStyle = circleGradient;
        this.ctx.fill();

        this.ctx.restore();
    }

    drawHandsCenter() {
        // 缩小中心圆形
        const centerRadius = 4;
        
        const gradient = this.ctx.createRadialGradient(
            this.centerX, this.centerY, 0,
            this.centerX, this.centerY, centerRadius
        );
        gradient.addColorStop(0, '#e8e8e8');  // 亮银色
        gradient.addColorStop(0.5, '#999999');  // 中银色
        gradient.addColorStop(1, '#666666');  // 暗银色

        this.ctx.beginPath();
        this.ctx.arc(this.centerX, this.centerY, centerRadius, 0, 2 * Math.PI);
        this.ctx.fillStyle = gradient;
        this.ctx.fill();
    }

    update() {
        const now = new Date();
        const hours = now.getHours();
        const minutes = now.getMinutes();
        const seconds = now.getSeconds();
        const milliseconds = now.getMilliseconds();

        this.frameCount++;
        
        requestAnimationFrame(() => this.update());
        
        // 提高刷新率以使动画更流畅
        const currentTime = Date.now();
        if (currentTime - this.lastTime >= 1000 / 60) {  // 改回60fps
            this.drawFace();
            this.drawDateWindow();
            this.drawHands(hours, minutes, seconds, milliseconds);
            this.lastTime = currentTime;
        }
    }

    start() {
        this.update();
    }

    // 添加颜色过渡计算方法
    getTransitionColor(progress) {
        // 定义过渡颜色数组
        const colors = [
            { r: 0, g: 0, b: 0 },        // 黑色
            { r: 64, g: 64, b: 64 },     // 深灰色
            { r: 128, g: 128, b: 128 },  // 中灰色
            { r: 192, g: 192, b: 192 },  // 浅灰色
            { r: 255, g: 255, b: 255 }   // 白色
        ];
        
        // 计算当前位置应该使用哪两个颜色
        const colorIndex = Math.min(Math.floor(progress * (colors.length - 1)), colors.length - 2);
        const colorProgress = (progress * (colors.length - 1)) - colorIndex;
        
        // 在两个颜色之间进行插
        const color1 = colors[colorIndex];
        const color2 = colors[colorIndex + 1];
        
        return {
            r: Math.round(color1.r + (color2.r - color1.r) * colorProgress),
            g: Math.round(color1.g + (color2.g - color1.g) * colorProgress),
            b: Math.round(color1.b + (color2.b - color1.b) * colorProgress)
        };
    }

    getRGBAString(color, alpha) {
        return `rgba(${color.r}, ${color.g}, ${color.b}, ${alpha})`;
    }
}

// 初始化时钟
document.addEventListener('DOMContentLoaded', () => {
    const canvas = document.getElementById('clockCanvas');
    if (canvas) {
        new LuxuryClock(canvas);
    }
});