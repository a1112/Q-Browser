import QtQuick
import QtTest

TestCase {
    id: test
    name: "QmlDemos"
    when: windowShown
    width: 1100; height: 720
    Component {
        id: fakeRuntimeComponent
        QtObject {
            property bool active: false
            property var calls: []
            property string pageTitle: ""
            property string pageStatus: ""
            signal capabilityFinished(string id, var response)
            function invoke(capability, operation, payload) {
                let id = String(calls.length + 1)
                calls.push({id: id, operation: operation, payload: payload})
                return id
            }
            function setPageMetadata(title, status) {
                if (status !== "loading" && status !== "ready") return false
                pageTitle = title
                pageStatus = status
                return true
            }
        }
    }
    function test_audioRequestsAreSerialized() {
        let runtime = createTemporaryObject(fakeRuntimeComponent, test)
        let component = Qt.createComponent("../../packages/elisa/qml/Main.qml")
        let page = createTemporaryObject(component, test, {runtime: runtime})
        compare(runtime.calls.length, 1)
        page.call("status")
        page.call("play", {trackId: "morning"})
        page.call("seek", {position: 1000})
        page.call("seek", {position: 2000})
        compare(runtime.calls.length, 1)
        runtime.capabilityFinished("1", {ok: true, result: {tracks: []}})
        tryVerify(() => runtime.calls.length === 2)
        compare(runtime.calls.length, 2)
        compare(runtime.calls[1].operation, "play")
        runtime.capabilityFinished("2", {ok: true, result: {state: "playing"}})
        tryVerify(() => runtime.calls.length === 3)
        compare(runtime.calls.length, 3)
        compare(runtime.calls[2].operation, "seek")
        compare(runtime.calls[2].payload.position, 2000)
    }
    function test_load_data() {
        return [{tag: "elisa"}, {tag: "tokodon"}, {tag: "coffee"}]
    }
    function test_load(data) {
        let component = Qt.createComponent("../../packages/" + data.tag + "/qml/Main.qml")
        compare(component.status, Component.Ready, component.errorString())
        let runtime = createTemporaryObject(fakeRuntimeComponent, test)
        let page = createTemporaryObject(component, test, {runtime: runtime})
        verify(page !== null)
        wait(250)
        compare(page.width > 0, true)
        compare(runtime.pageStatus, "ready")
        verify(runtime.pageTitle.length > 0)
    }
    function test_coffeeWorkflow() {
        let component = Qt.createComponent("../../packages/coffee/qml/Main.qml")
        let page = createTemporaryObject(component, test)
        verify(page !== null)
        let flow = findChild(page, "coffee-flow")
        verify(flow !== null)
        compare(flow.state, "Home")
        flow.cappuccino()
        compare(flow.state, "Settings")
        flow.confirmButton()
        compare(flow.state, "Insert")
        flow.continueButton()
        compare(flow.state, "Progress")
        tryCompare(flow, "state", "制作完成", 7000)
        flow.onReturnToStart()
        compare(flow.state, "Home")
    }
    function test_tokodonStateIsLocal() {
        let component = Qt.createComponent("../../packages/tokodon/qml/Main.qml")
        let first = createTemporaryObject(component, test)
        let second = createTemporaryObject(component, test)
        let posts = findChild(first, "posts")
        let other = findChild(second, "posts")
        compare(posts.count, 3)
        first.toggle(0, "liked")
        first.toggle(0, "saved")
        compare(posts.get(0).liked, true)
        compare(posts.get(0).saved, true)
        compare(other.get(0).liked, false)
        posts.setProperty(0, "replies", "本地回复")
        compare(other.get(0).replies, "")
    }
}
