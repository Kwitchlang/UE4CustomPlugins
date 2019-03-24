from functools import partial
import maya.cmds as cmds
winID = 'kevsUI'
windowwidth = 400
windowHeight = 300
MainWindows = cmds.window(w=windowwidth,h=windowHeight,title="EA to Unreal Engine")
rowwidth = 100



buttonForm = cmds.formLayout( parent = MainWindows )

allowedAreas = ['all']
cmds.dockControl( area='right', content=MainWindows, allowedArea=allowedAreas )



SelectedObjects = cmds.ls(selection=True)
if cmds.window(winID, exists=True):
    cmds.deleteUI(winID)



column = cmds.columnLayout("Master",w=windowwidth)
#cmds.gridLayout("MasterGrid",numberOfColumns=1,cellWidthHeight=(windowwidth,45));
cmds.text(l="Material and LOD Tools:", al='center')
buttonwidth = 400
cmds.button(l = "Create LODS from Selection", w = buttonwidth, c = "CreateLODs ()", al = "center")
cmds.button(l = "Create Material from Selection", w = buttonwidth, c = "CreateMaterials ()", al = "center")
cmds.button(l = "Special Menu", w = buttonwidth, c = "OpenSpecial ()", al = "center")


cmds.text(l="Pivot Tools:")




ColNumber =2;
cmds.gridLayout("ToolLayout",numberOfColumns=ColNumber,cellWidthHeight=(windowwidth/ColNumber,45));

Colwidth = 40
cmds.button(label = "Set Pivot to bounding box", w = buttonwidth, c = "SetPivotBoundingBox (xPlus,yPlus,zPlus,xMinus,yMinus,zMinus)")
cmds.rowColumnLayout('table', numberOfColumns=4, columnAttach=(50, 'left', 1), columnWidth=[(1, Colwidth), (2, Colwidth),(3, Colwidth),(4, Colwidth) ])

cmds.text(parent = 'table',label="+")
xPlus = cmds.checkBox(parent = 'table',label = "X")
yPlus = cmds.checkBox(parent = 'table',label = "Y")
zPlus = cmds.checkBox(parent = 'table',label = "Z")
cmds.text(parent = 'table',label="-")
xMinus = cmds.checkBox(parent = 'table',label = "X")
yMinus = cmds.checkBox(parent = 'table',label = "Y")
zMinus = cmds.checkBox(parent = 'table',label = "Z")

cmds.button(parent ='ToolLayout', label = "Move Object to World Origin",w = buttonwidth, c = "SetObjectToOrigin (CheckFreeze)")
CheckFreeze = cmds.checkBox(parent ='ToolLayout',label = "Freeze Transform")


cmds.gridLayout("Locator Options",parent="Master",numberOfColumns=1,cellWidthHeight=(windowwidth,45));
cmds.text(l="Locator Mode")
cmds.button( label = "Open Locator mode",w = buttonwidth, c = "OpenLocatorMode ()")





def OpenLocatorMode():
    cmds.selectMode (component=True)
    window = cmds.window(w=200, h = 75)
    cmds.rowColumnLayout( numberOfColumns=1, columnAttach=(100, 'left', 1), columnWidth=[(1, buttonwidth), (2, buttonwidth)] )    
    
    socketoption = cmds.optionMenu( label='Socket Type:' )
    cmds.button(label = "Add Locator/UE Socket:", w = 300, command = lambda *args: LocatorAtEdge( cmds.optionMenu( socketoption,q=1,v=1) ))
    
   
##################################################################  
#        Added Entries for Socket generation
    cmds.showWindow( window )
    
    cmds.menuItem( label='Camera Position',c = "PrintMenu ()" )
    cmds.menuItem( label='Hover Component',c = "PrintMenu ()" )
    cmds.menuItem( label='Muzzle Position',c = "PrintMenu ()" )
    cmds.menuItem( label='Left Hand',c = "PrintMenu ()" )
    cmds.menuItem( label='Right Hand',c = "PrintMenu ()" )
    cmds.menuItem( label='Attachment Location',c = "PrintMenu ()" )
#####################################################################    
	



def LocatorAtEdge(option):
    # Take Edge Selection
    selectionoption = cmds.optionMenu(option)
    sel = cmds.ls(selection = True, flatten = True)
    print (option)
    
    #Check to see if there is a selecti
    if sel:
        for edge in sel:        
            #Convert selection to vertices
            #polyListComponentConversion it returns verts grouped
            #usinf mc.ls to flatten to get each vertice  
            verts = cmds.ls(cmds.polyListComponentConversion(edge, toVertex = True), flatten = True)
            #print verts
            
            for v in verts:
                #Get Individual world positions of each vert
                a = cmds.xform(verts[0], query = True, translation = True, worldSpace = True)
                b = cmds.xform(verts[1], query = True, translation = True, worldSpace = True)
                
                #Math to get the position between both vertices        
                x = (b[0] + a[0]) / 2
                y = (b[1] + a[1]) / 2
                z = (b[2] + a[2]) / 2
                
                #absolute means it will be in worldspace
                # Strips Selection option of any Spaces
            cmds.spaceLocator(absolute = True, name = "SOCKET_" + option.replace(" ", "") + "_0", position = (x,y,z))        
    cmds.select(sel, replace = True)       

def SetObjectToOrigin(CheckFreeze):
    CheckFreeze = cmds.checkBox(CheckFreeze,query = True, value = True) 
    cmds.makeIdentity(apply=True, t=1, r=1, s=1, n=0);
    pos = cmds.xform(q=True ,piv=True,ws=True);
    cmds.move(0,0,0,rotatePivotRelative=True);
    if CheckFreeze==True:
        cmds.makeIdentity(apply=True, t=1, r=1, s=1, n=0)

def CreateLODs():
    selected = cmds.ls(sl=1)
    cmds.LevelOfDetailGroup()
    
    
def CreateMaterials():
    LODSelection1 = cmds.ls(selection=True,o=1)
    ObjectName = LODSelection1;
    for SingleObject in LODSelection1:
        splitby = SingleObject.split("|")
        GetLastName=splitby[len(splitby)-1] #Get last index
        print GetLastName
        cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name = GetLastName + 'SG')
        cmds.shadingNode('blinn', asShader=True, name=GetLastName)
        cmds.connectAttr(GetLastName + '*' + '.outColor', GetLastName + 'SG.surfaceShader')
        LookforSimilar = cmds.select("*" +  GetLastName + "*", r=True)
        selected = cmds.ls(sl=1)
        print LookforSimilar
        for similarObjects in selected:
            cmds.sets( similarObjects, e=True, forceElement = GetLastName + 'SG' )
            
            
def OpenSpecial():
    window = cmds.window(w=300)
    cmds.rowColumnLayout( numberOfColumns=1, columnAttach=(100, 'left', 1), columnWidth=[(1, buttonwidth), (2, buttonwidth)] )    
    textfield = cmds.textField()
    cmds.button(label = "Search and delete with:", w = 300, command = partial (SearchandDelete, textfield))    
    cmds.showWindow( window )
    
    
def SearchandDelete(input,*args):
    TextInput = cmds.textField( input, q=True, text=True) 
    cmds.select ("*" + TextInput + "*")
    cmds.ls (sl=1,o=1)
    cmds.delete()
    
    
def SetPivotBoundingBox(xPlus,yPlus,zPlus,xMinus,yMinus,zMinus):
    selectionList = cmds.ls( orderedSelection=True );
    if len( selectionList ) == 1:
        targetName = selectionList[0];
        #cmds.makeIdentity(apply=True, t=1, r=1, s=1, n=0);
        boundingBox = cmds.xform(targetName, q=True, bb=True)
        #    0     1     2     3     4     5
        #   xmin, ymin, zmin, xmax, ymax zmax.
        xPlusBool = cmds.checkBox(xPlus,query = True, value = True)  
        yPlusBool = cmds.checkBox(yPlus,query = True, value = True)  
        zPlusBool = cmds.checkBox(zPlus,query = True, value = True)  
        xMinusBool = cmds.checkBox(xMinus,query = True, value = True) 
        yMinusBool = cmds.checkBox(yMinus,query = True, value = True) 
        zMinusBool = cmds.checkBox(zMinus,query = True, value = True)   
        
        xLoc = returnValue(boundingBox,xMinusBool,xPlusBool,0,3)
        yLoc = returnValue(boundingBox,yMinusBool,yPlusBool,1,4)
        zLoc = returnValue(boundingBox,zMinusBool,zPlusBool,2,5)
        print xLoc
        print yLoc
        print zLoc
        
        cmds.move(xLoc, yLoc, zLoc, targetName + '.rotatePivot');
        cmds.move(xLoc, yLoc, zLoc, targetName + '.scalePivot');

        
        
def returnValue(boundingBox,minusBool,posBool,minIndex,maxIndex):
    if  posBool==False and minusBool==True:
        print 'Negitive Bool'
        outputvalue = boundingBox[minIndex]
   
    if minusBool==False and posBool==True:
        outputvalue = boundingBox[maxIndex]
        print 'Positive Bool'
        
    if minusBool == True and posBool == True or minusBool == False and posBool == False:
        outputvalue =  (boundingBox[maxIndex] + boundingBox[minIndex]) /2
        print 'Center'
        
    return outputvalue
         
cmds.showWindow( window )