import tkinter as tk
from tkinter import simpledialog, filedialog, messagebox

# terrain types
DRAWABLES = {
    " ": "Out of Bounds",
    "#": "Terrain",
    "W": "Wall",
   "S": "Slime"
}

class MapEditor:
    def __init__(self, root):
        self.root = root
        self.root.withdraw()  # hide app on init
        self.cell_size = 20
        self.map_data = []
        self.current_tool = "#"  # default is terrain
        self.is_drawing = False
        self.tool_buttons = {} #button refs

        # dimensions from user
        self.rows = simpledialog.askinteger("Input", "Enter map height (rows):", minvalue=1, parent=self.root)
        self.columns = simpledialog.askinteger("Input", "Enter map width (columns):", minvalue=1, parent=self.root)

        if not self.rows or not self.columns:
            messagebox.showerror("Error", "Invalid dimensions!")
            self.root.destroy()
            return

        self.root.deiconify()  # show main window
        self.init_map_data()
        self.create_widgets()

    def init_map_data(self):
      #empty map
        self.map_data = [[" " for _ in range(self.columns)] for _ in range(self.rows)]

    def create_widgets(self):
        # frame forbtns + canvas
        self.frame = tk.Frame(self.root)
        self.frame.grid(row=0, column=0, columnspan=2, sticky="nsew")

        # map canvas
        self.canvas = tk.Canvas(self.frame, width=min(600, self.columns * self.cell_size),
                                height=min(400, self.rows * self.cell_size), bg="white")

        # scrollbars
        self.h_scrollbar = tk.Scrollbar(self.frame, orient="horizontal", command=self.canvas.xview)
        self.h_scrollbar.pack(side="bottom", fill="x")
        self.v_scrollbar = tk.Scrollbar(self.frame, orient="vertical", command=self.canvas.yview)
        self.v_scrollbar.pack(side="right", fill="y")

        # canwas scrollbars config
        self.canvas.configure(xscrollcommand=self.h_scrollbar.set, yscrollcommand=self.v_scrollbar.set)
        self.canvas.pack(side="left", fill="both", expand=True)

        # big scroll canvas
        self.canvas.config(scrollregion=(0, 0, self.columns * self.cell_size, self.rows * self.cell_size))

        # mouse binds for painting
        self.canvas.bind("<B1-Motion>", self.paint)
        self.canvas.bind("<ButtonPress-1>", self.start_paint)
        self.canvas.bind("<ButtonRelease-1>", self.stop_paint)

        # buttons for gridcell types
        self.button_frame = tk.Frame(self.root)
        self.button_frame.grid(row=1, column=0, columnspan=2, sticky="ew")
        
        for symbol, name in DRAWABLES.items():
            button = tk.Button(self.button_frame, text=name, command=lambda s=symbol: self.set_tool(s))
            button.pack(side="left", padx=2, pady=2)
            self.tool_buttons[symbol] = button  # save button

        # current button
        self.highlight_selected_tool()

        # export btn
        self.export_button = tk.Button(self.root, text="Export to .txt", command=self.export_map)
        self.export_button.grid(row=2, column=0, columnspan=2, sticky="ew")

        self.draw_grid()

    def draw_grid(self):
        self.canvas.delete("all")
        for r in range(self.rows):
            for c in range(self.columns):
                x1 = c * self.cell_size
                y1 = r * self.cell_size
                x2 = x1 + self.cell_size
                y2 = y1 + self.cell_size
                #coloring in for the different types
                fill_color = "white" if self.map_data[r][c] == " " else "green"
                if self.map_data[r][c] == "W":
                    fill_color = "gray"
                elif self.map_data[r][c] == "S":
                    fill_color = "yellow"
                self.canvas.create_rectangle(x1, y1, x2, y2, fill=fill_color, outline="black")

    def set_tool(self, tool):
        self.current_tool = tool
        self.highlight_selected_tool()

    def highlight_selected_tool(self):
        for symbol, button in self.tool_buttons.items():
            if symbol == self.current_tool:
                button.config(bg="lightblue")  # highlight current btn
            else:
                button.config(bg="gray85")  # needs a color cause Linux


    def start_paint(self, event):
        self.is_drawing = True
        self.paint(event)

    def stop_paint(self, event):
        self.is_drawing = False

    def paint(self, event):
        if not self.is_drawing:
            return

        # cell coords
        col = int((self.canvas.canvasx(event.x)) // self.cell_size)
        row = int((self.canvas.canvasy(event.y)) // self.cell_size)

        # within bounds?
        if 0 <= row < self.rows and 0 <= col < self.columns:
            self.map_data[row][col] = self.current_tool
            self.draw_grid()

    def export_map(self):
        # whatever Data[x][Y] format is, i can export to that to a file
        file_path = filedialog.asksaveasfilename(defaultextension=".txt", filetypes=[("Text files", "*.txt")])
        if file_path:
            try:
                with open(file_path, 'w') as f:
                    f.write(f"char Data[{self.rows}][{self.columns}] = {{\n")
                    for r in range(self.rows):
                        row_data = ", ".join(f"'{self.map_data[r][c]}'" for c in range(self.columns))
                        f.write(f"\t{row_data},\n")
                    f.write("};\n")
                messagebox.showinfo("Success", "Map exported successfully!")
            except Exception as e:
                messagebox.showerror("Error", f"Failed to save file: {e}")

if __name__ == "__main__":
    root = tk.Tk()
    app = MapEditor(root)
    root.mainloop()
