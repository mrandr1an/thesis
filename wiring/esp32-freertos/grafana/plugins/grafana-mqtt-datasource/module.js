define(["@grafana/data","@grafana/runtime","react","@grafana/ui","@emotion/css","lodash"],((e,t,r,n,a,l)=>(()=>{"use strict";var o={89:e=>{e.exports=a},781:t=>{t.exports=e},531:e=>{e.exports=t},7:e=>{e.exports=n},241:e=>{e.exports=l},959:e=>{e.exports=r}},i={};function c(e){var t=i[e];if(void 0!==t)return t.exports;var r=i[e]={exports:{}};return o[e](r,r.exports,c),r.exports}c.n=e=>{var t=e&&e.__esModule?()=>e.default:()=>e;return c.d(t,{a:t}),t},c.d=(e,t)=>{for(var r in t)c.o(t,r)&&!c.o(e,r)&&Object.defineProperty(e,r,{enumerable:!0,get:t[r]})},c.o=(e,t)=>Object.prototype.hasOwnProperty.call(e,t),c.r=e=>{"undefined"!=typeof Symbol&&Symbol.toStringTag&&Object.defineProperty(e,Symbol.toStringTag,{value:"Module"}),Object.defineProperty(e,"__esModule",{value:!0})};var s={};c.r(s),c.d(s,{plugin:()=>ee});var u=c(781),p=c(531);function d(e,t,r){return t in e?Object.defineProperty(e,t,{value:r,enumerable:!0,configurable:!0,writable:!0}):e[t]=r,e}class m extends p.DataSourceWithBackend{applyTemplateVariables(e,t){let r=(0,p.getTemplateSrv)().replace(e.topic,t);r=r.replace(/\+/gi,"__PLUS__"),r=r.replace(/\#/gi,"__HASH__");const n=(a=function(e){for(var t=1;t<arguments.length;t++){var r=null!=arguments[t]?arguments[t]:{},n=Object.keys(r);"function"==typeof Object.getOwnPropertySymbols&&(n=n.concat(Object.getOwnPropertySymbols(r).filter((function(e){return Object.getOwnPropertyDescriptor(r,e).enumerable})))),n.forEach((function(t){d(e,t,r[t])}))}return e}({},e),l=null!=(l={topic:r})?l:{},Object.getOwnPropertyDescriptors?Object.defineProperties(a,Object.getOwnPropertyDescriptors(l)):function(e){var t=Object.keys(e);if(Object.getOwnPropertySymbols){var r=Object.getOwnPropertySymbols(e);t.push.apply(t,r)}return t}(Object(l)).forEach((function(e){Object.defineProperty(a,e,Object.getOwnPropertyDescriptor(l,e))})),a);var a,l;return n}constructor(e){super(e)}}var f=c(959),h=c.n(f),b=c(89),g=c(7),y=Object.defineProperty,E=Object.defineProperties,O=Object.getOwnPropertyDescriptors,v=Object.getOwnPropertySymbols,C=Object.prototype.hasOwnProperty,w=Object.prototype.propertyIsEnumerable,S=(e,t,r)=>t in e?y(e,t,{enumerable:!0,configurable:!0,writable:!0,value:r}):e[t]=r,P=(e,t)=>{for(var r in t||(t={}))C.call(t,r)&&S(e,r,t[r]);if(v)for(var r of v(t))w.call(t,r)&&S(e,r,t[r]);return e};const T=({dataSourceName:e,docsLink:t,hasRequiredFields:r=!0,className:n})=>{const a=(0,g.useTheme2)(),l={container:(0,b.css)({p:{margin:0},"p + p":{marginTop:a.spacing(2)}}),text:(0,b.css)((o=P({},a.typography.body),i={color:a.colors.text.secondary,a:(0,b.css)({color:a.colors.text.link,textDecoration:"underline","&:hover":{textDecoration:"none"}})},E(o,O(i))))};var o,i;return h().createElement("div",{className:(0,b.cx)(l.container,n)},h().createElement("p",{className:l.text},"Before you can use the ",e," data source, you must configure it below or in the config file. For detailed instructions,"," ",h().createElement("a",{href:t,target:"_blank",rel:"noreferrer"},"view the documentation"),"."),r&&h().createElement("p",{className:l.text},h().createElement("i",null,"Fields marked with * are required")))};var j=Object.defineProperty,x=Object.defineProperties,D=Object.getOwnPropertyDescriptors,A=Object.getOwnPropertySymbols,I=Object.prototype.hasOwnProperty,F=Object.prototype.propertyIsEnumerable,k=(e,t,r)=>t in e?j(e,t,{enumerable:!0,configurable:!0,writable:!0,value:r}):e[t]=r,N=(e,t)=>{for(var r in t||(t={}))I.call(t,r)&&k(e,r,t[r]);if(A)for(var r of A(t))F.call(t,r)&&k(e,r,t[r]);return e};const R=({children:e,title:t,description:r,isCollapsible:n=!1,isInitiallyOpen:a=!0,kind:l="section",className:o})=>{const{colors:i,typography:c,spacing:s}=(0,g.useTheme2)(),[u,p]=(0,f.useState)(!n||a),d=u?"angle-up":"angle-down",m="sub-section"===l,y=`${u?"Collapse":"Expand"} section ${t}`,E={header:(0,b.css)({display:"flex",justifyContent:"space-between",alignItems:"center"}),title:(0,b.css)({margin:0}),subtitle:(0,b.css)({margin:0,fontWeight:c.fontWeightRegular}),descriptionText:(0,b.css)((O=N({marginTop:s(m?.25:.5),marginBottom:0},c.bodySmall),v={color:i.text.secondary},x(O,D(v)))),content:(0,b.css)({marginTop:s(2)})};var O,v;return h().createElement("div",{className:o},h().createElement("div",{className:E.header},"section"===l?h().createElement("h3",{className:E.title},t):h().createElement("h6",{className:E.subtitle},t),n&&h().createElement(g.IconButton,{name:d,onClick:()=>p(!u),type:"button",size:"xl","aria-label":y})),r&&h().createElement("p",{className:E.descriptionText},r),u&&h().createElement("div",{className:E.content},e))};var L=Object.defineProperty,W=Object.defineProperties,_=Object.getOwnPropertyDescriptors,U=Object.getOwnPropertySymbols,J=Object.prototype.hasOwnProperty,B=Object.prototype.propertyIsEnumerable,q=(e,t,r)=>t in e?L(e,t,{enumerable:!0,configurable:!0,writable:!0,value:r}):e[t]=r;const K=e=>{var t,r=e,{children:n}=r,a=((e,t)=>{var r={};for(var n in e)J.call(e,n)&&t.indexOf(n)<0&&(r[n]=e[n]);if(null!=e&&U)for(var n of U(e))t.indexOf(n)<0&&B.call(e,n)&&(r[n]=e[n]);return r})(r,["children"]);return h().createElement(R,(t=((e,t)=>{for(var r in t||(t={}))J.call(t,r)&&q(e,r,t[r]);if(U)for(var r of U(t))B.call(t,r)&&q(e,r,t[r]);return e})({},a),W(t,_({kind:"section"}))),n)},z=()=>{const e=(0,g.useStyles2)(G);return h().createElement("hr",{className:e.horizontalDivider})},G=e=>({horizontalDivider:(0,b.css)({borderTop:`1px solid ${e.colors.border.weak}`,margin:e.spacing(2,0),width:"100%"})});var M=Object.getOwnPropertySymbols,V=Object.prototype.hasOwnProperty,Q=Object.prototype.propertyIsEnumerable;const $=e=>{var t=e,{children:r}=t,n=((e,t)=>{var r={};for(var n in e)V.call(e,n)&&t.indexOf(n)<0&&(r[n]=e[n]);if(null!=e&&M)for(var n of M(e))t.indexOf(n)<0&&Q.call(e,n)&&(r[n]=e[n]);return r})(t,["children"]);const a=(0,g.useStyles2)((0,f.useCallback)((e=>H(e,n)),[n]));return h().createElement("div",{className:a.root},r)},H=(e,t)=>{var r,n,a;return{root:(0,b.css)({display:"flex",flexDirection:null!=(r=t.direction)?r:"row",flexWrap:null==(n=t.wrap)||n?"wrap":void 0,alignItems:t.alignItems,gap:e.spacing(null!=(a=t.gap)?a:2),flexGrow:t.flexGrow})}},Y=e=>{const{editorProps:t,showCACert:r,showKeyPair:n=!0}=e,{secureJsonFields:a}=t.options;return h().createElement(h().Fragment,null,r?h().createElement(g.Field,{label:h().createElement(g.Label,null,h().createElement($,{gap:.5},h().createElement("span",null,"TLS CA Certificate"),h().createElement(g.Tooltip,{content:h().createElement("span",null,"If a Certificate Authority certificate is required to verify the server's certificate, provide it here.")},h().createElement(g.Icon,{name:"info-circle",size:"sm"}))))},h().createElement(g.SecretTextArea,{placeholder:"-----BEGIN CERTIFICATE-----",cols:45,rows:7,isConfigured:a&&a.tlsCACert,onChange:(0,u.onUpdateDatasourceSecureJsonDataOption)(t,"tlsCACert"),onReset:()=>{(0,u.updateDatasourcePluginResetOption)(t,"tlsCACert")}})):null,n?h().createElement(g.Field,{label:h().createElement(g.Label,null,h().createElement($,{gap:.5},h().createElement("span",null,"TLS Client Certificate"),h().createElement(g.Tooltip,{content:h().createElement("span",null,"To authenticate with an TLS client certificate, provide the client certificate here.")},h().createElement(g.Icon,{name:"info-circle",size:"sm"}))))},h().createElement(g.SecretTextArea,{placeholder:"-----BEGIN CERTIFICATE-----",cols:45,rows:7,isConfigured:a&&a.tlsClientCert,onChange:(0,u.onUpdateDatasourceSecureJsonDataOption)(t,"tlsClientCert"),onReset:()=>{(0,u.updateDatasourcePluginResetOption)(t,"tlsClientCert")}})):null,n?h().createElement(g.Field,{label:h().createElement(g.Label,null,h().createElement($,{gap:.5},h().createElement("span",null,"TLS Client Key"),h().createElement(g.Tooltip,{content:h().createElement("span",null,"To authenticate with a client TLS certificate, provide the private key here.")},h().createElement(g.Icon,{name:"info-circle",size:"sm"}))))},h().createElement(g.SecretTextArea,{placeholder:"-----BEGIN RSA PRIVATE KEY-----",cols:45,rows:7,isConfigured:a&&a.tlsClientKey,onChange:(0,u.onUpdateDatasourceSecureJsonDataOption)(t,"tlsClientKey"),onReset:()=>{(0,u.updateDatasourcePluginResetOption)(t,"tlsClientKey")}})):null)};var X=c(241),Z=c.n(X);const ee=new u.DataSourcePlugin(m).setConfigEditor((e=>{const{options:t}=e,r=t.jsonData,n=t=>r=>{(0,u.updateDatasourcePluginJsonDataOption)(e,t,r.currentTarget.checked)};return h().createElement(h().Fragment,null,h().createElement(T,{dataSourceName:"MQTT",docsLink:"https://grafana.com/grafana/plugins/grafana-mqtt-datasource/?tab=overview",hasRequiredFields:!0}),h().createElement(z,null),h().createElement(K,{title:"Connection"},h().createElement(g.Field,{label:"URI",required:!0},h().createElement(g.Input,{width:40,name:"URI",type:"text",value:r.uri||"",onChange:(0,u.onUpdateDatasourceJsonDataOption)(e,"uri"),placeholder:"TCP (tcp://), TLS (tls://), or WebSocket (ws://)"}))),h().createElement(z,null),h().createElement(K,{title:"Authentication"},h().createElement(g.Field,{label:"Username"},h().createElement(g.Input,{width:40,value:r.username||"",placeholder:"Username",onChange:(0,u.onUpdateDatasourceJsonDataOption)(e,"username")})),h().createElement(g.Field,{label:"Password"},h().createElement(g.SecretInput,{width:40,placeholder:"Password",isConfigured:t.secureJsonFields&&t.secureJsonFields.password,onReset:()=>{(0,u.updateDatasourcePluginResetOption)(e,"password")},onBlur:(0,u.onUpdateDatasourceSecureJsonDataOption)(e,"password")})),h().createElement(g.Field,{label:"Use TLS Client Auth",description:"Enables TLS authentication using client cert configured in secure json data."},h().createElement(g.Switch,{onChange:n("tlsAuth"),value:r.tlsAuth||!1})),h().createElement(g.Field,{label:"Skip TLS Verification",description:"When enabled, skips verification of the MQTT server's TLS certificate chain and host name."},h().createElement(g.Switch,{onChange:n("tlsSkipVerify"),value:r.tlsSkipVerify||!1})),h().createElement(g.Field,{label:"With CA Cert",description:"Needed for verifying servers with self-signed TLS Certs."},h().createElement(g.Switch,{onChange:n("tlsAuthWithCACert"),value:r.tlsAuthWithCACert||!1}))),r.tlsAuth||r.tlsAuthWithCACert?h().createElement(h().Fragment,null,h().createElement(z,null),h().createElement(K,{title:"TLS Configuration"},r.tlsAuth||r.tlsAuthWithCACert?h().createElement(Y,{showCACert:r.tlsAuthWithCACert,showKeyPair:r.tlsAuth,editorProps:e,labelWidth:40}):null)):null)})).setQueryEditor((e=>{const{query:t,onChange:r,onRunQuery:n}=e,a=((e,t)=>(r,n=e=>e)=>a=>{const l=Z().cloneDeep(e);t(Z().set(l,r,n(a.currentTarget.value)))})(t,r);return h().createElement(h().Fragment,null,h().createElement(g.InlineFieldRow,null,h().createElement(g.InlineField,{label:"Topic",labelWidth:8,grow:!0},h().createElement(g.Input,{name:"topic",required:!0,placeholder:'e.g. "home/bedroom/temperature"',value:t.topic,onBlur:n,onChange:a("topic")}))))}));return s})()));
//# sourceMappingURL=module.js.map