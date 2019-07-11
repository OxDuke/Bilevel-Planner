'   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
'
'   File:    lo1.vb
'
'   Purpose: Demonstrates how to solve small linear
'            optimization problem using the MOSEK .net API.
'
Imports System, mosek


Public Class MsgTest
    Inherits mosek.Stream
    Dim name As String

    Public Sub New(ByVal e As mosek.Env, ByVal n As String)
        '        MyBase.New ()
        name = n
    End Sub

    Public Overrides Sub streamCB(ByVal msg As String)
        Console.Write("{0}: {1}", name, msg)
    End Sub
End Class

Module Module1
    Sub Main()
        Dim infinity As Double = 0.0
        Dim numcon As Integer = 3
        Dim numvar As Integer = 4

        Dim bkc As boundkey() = {boundkey.fx, boundkey.lo, boundkey.up}
        Dim bkx As boundkey() = {boundkey.lo, boundkey.ra, boundkey.lo, boundkey.lo}
        Dim asub(numvar)() As Integer
        asub(0) = New Integer() {0, 1}
        asub(1) = New Integer() {0, 1, 2}
        asub(2) = New Integer() {0, 1}
        asub(3) = New Integer() {1, 2}

        Dim blc As Double() = {30.0, 15.0, -infinity}
        Dim buc As Double() = {30.0, infinity, 25.0}
        Dim cj As Double() = {3.0, 1.0, 5.0, 1.0}
        Dim blx As Double() = {0.0, 0.0, 0.0, 0, 0}
        Dim bux As Double() = {infinity, 10, infinity, infinity}

        Dim aval(numvar)() As Double
        aval(0) = New Double() {3.0, 2.0}
        aval(1) = New Double() {1.0, 1.0, 2.0}
        aval(2) = New Double() {2.0, 3.0}
        aval(3) = New Double() {1.0, 3.0}

        Dim xx As Double() = {0, 0, 0, 0, 0}

        Dim msg As MsgTest
        Dim i As Integer
        Dim j As Integer

        Try
            Using env As New mosek.Env()
                Using task As New mosek.task(env, 0, 0)
                    msg = New MsgTest(env, "msg")
                    task.set_Stream(streamtype.log, msg)

                    'Append 'numcon' empty constraints.
                    'The constraints will initially have no bounds. 
                    Call task.appendcons(numcon)

                    'Append 'numvar' variables.
                    ' The variables will initially be fixed at zero (x=0). 

                    Call task.appendvars(numvar)

                    For j = 0 To numvar - 1
                        'Set the linear term c_j in the objective.
                        Call task.putcj(j, cj(j))

                        ' Set the bounds on variable j.
                        'blx[j] <= x_j <= bux[j] 
                        Call task.putvarbound(j, bkx(j), blx(j), bux(j))
                        'Input column j of A 
                        Call task.putacol(j, asub(j), aval(j))
                    Next j

                    ' for i=1, ...,numcon : blc[i] <= constraint i <= buc[i] 
                    For i = 0 To numcon - 1
                        Call task.putconbound(i, bkc(i), blc(i), buc(i))
                    Next i

                    Call task.putobjsense(mosek.objsense.maximize)

                    Call task.optimize()
                    ' Print a summary containing information
                    '   about the solution for debugging purposes
                    Call task.solutionsummary(mosek.streamtype.msg)

                    Dim solsta As mosek.solsta
                    ' Get status information about the solution 

                    Call task.getsolsta(mosek.soltype.bas, solsta)

                    task.getxx(soltype.bas, xx)

                    For j = 0 To numvar - 1
                        Console.WriteLine("x[{0}]:{1}", j, xx(j))
                    Next

                    Console.WriteLine("Finished optimization")
                End Using
            End Using
        Catch e As mosek.Exception
            Console.WriteLine("MosekException caught, {0}", e)
            Throw (e)
        End Try
    End Sub
End Module