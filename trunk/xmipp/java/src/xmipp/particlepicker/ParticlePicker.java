package xmipp.particlepicker;

import ij.CommandListener;
import ij.Executer;
import ij.ImageListener;
import ij.ImagePlus;
import ij.gui.SaveChangesDialog;
import ij.plugin.frame.Recorder;
import java.awt.Color;
import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.logging.FileHandler;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.logging.SimpleFormatter;

import xmipp.particlepicker.tiltpair.model.UntiltedMicrograph;
import xmipp.particlepicker.training.model.FamilyState;
import xmipp.particlepicker.training.model.MicrographFamilyData;
import xmipp.particlepicker.training.model.SupervisedParticlePicker;
import xmipp.particlepicker.training.model.TrainingPicker;
import xmipp.utils.XmippMessage;
import xmipp.jni.MDLabel;
import xmipp.jni.MetaData;
import xmipp.jni.Program;

public abstract class ParticlePicker
{

	protected String familiesfile;
	protected String macrosfile;
	protected static Logger logger;
	protected String outputdir = ".";
	protected boolean changed;
	protected List<Family> families;
	protected FamilyState mode;
	protected List<Filter> filters;
	protected String selfile;
	protected String command;
	protected Family family;
	
	public int getSize()
	{
		return family.getSize();
	}

	public Color getColor()
	{
		return family.getColor();
	}
	
	public void setColor(Color color)
	{
		family.setColor(color);
		setChanged(true);
	}
	
	public void setSize(int size)
	{
		family.setSize(size);
		setChanged(true);
	}

	public Family getFamily()
	{
		return family;
	}
	
	public void setFamily(Family family)
	{
		this.family = family;
		
	}


	public String getPosFileFromXmipp24Project(String projectdir, String mname)
	{
		String suffix = ".raw.Common.pos";
		return String.format("%1$s%2$sPreprocessing%2$s%3$s%2$s%3$s%4$s", projectdir, File.separator, mname, suffix);
	}

	public ParticlePicker(String selfile, String outputdir, FamilyState mode)
	{
		this.outputdir = outputdir;
		this.familiesfile = getOutputPath("families.xmd");
		
		this.families = new ArrayList<Family>();
		loadFamilies();
		family = families.get(0);

		this.selfile = selfile;
		this.mode = mode;
		initializeFilters();
	}

	public ParticlePicker(String selfile, String outputdir, String fname, FamilyState mode)
	{
		this.outputdir = outputdir;
		this.familiesfile = getOutputPath("families.xmd");
		this.families = new ArrayList<Family>();
		loadFamilies();
		family = getFamily(fname);
		if (family == null)
			throw new IllegalArgumentException("Invalid family " + fname);
		
		this.selfile = selfile;
		this.outputdir = outputdir;
		this.mode = mode;
		initializeFilters();

	}

	private void initializeFilters()
	{
		this.macrosfile = getOutputPath("macros.xmd");
		filters = new ArrayList<Filter>();
		loadFilters();
		Recorder.record = true;

		// detecting if a command is thrown by ImageJ
		Executer.addCommandListener(new CommandListener()
		{
			public String commandExecuting(String command)
			{
				ParticlePicker.this.command = command;
				System.out.println(command);
				return command;

			}
		});
		ImagePlus.addImageListener(new ImageListener()
		{

			@Override
			public void imageUpdated(ImagePlus arg0)
			{
				updateFilters();
			}

			@Override
			public void imageOpened(ImagePlus arg0)
			{

			}

			@Override
			public void imageClosed(ImagePlus arg0)
			{
				// TODO Auto-generated method stub

			}
		});
	}

	private void updateFilters()
	{
		System.out.println("Image updated");
		if (command != null)
		{
			String options = "";
			if (Recorder.getCommandOptions() != null)
				options = Recorder.getCommandOptions();
			if (!isFilterSelected(command))
				addFilter(command, options);
			else if (!(options == null || options.equals("")))
				for (Filter f : filters)
					if (f.getCommand().equals(command))
						f.setOptions(options);
			setChanged(true);
			command = null;

		}
	}

	public String getMicrographsSelFile()
	{
		return selfile;
	}

	void addFilter(String command, String options)
	{
		Filter f = new Filter(command, options);
		filters.add(f);
	}

	public List<Filter> getFilters()
	{
		return filters;
	}

	public void setChanged(boolean changed)
	{
		this.changed = changed;
	}

	public boolean isChanged()
	{
		return changed;
	}

	public FamilyState getMode()
	{
		return mode;
	}

	public static Logger getLogger()
	{
		try
		{
			if (logger == null)
			{
				FileHandler fh = new FileHandler("PPicker.log", true);
				fh.setFormatter(new SimpleFormatter());
				logger = Logger.getLogger("PPickerLogger");
				logger.addHandler(fh);
			}
			return logger;
		}
		catch (Exception e)
		{
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return null;
	}

	public String getOutputPath(String file)
	{
		return outputdir + File.separator + file;
	}

	public String getOutputDir()
	{
		return outputdir;
	}

	public List<Family> getFamilies()
	{
		return families;
	}

	public void persistFamilies()
	{
		long id;
		String file = familiesfile;
		try
		{
			MetaData md = new MetaData();
			for (Family f : families)
			{
				id = md.addObject();
				md.setValueString(MDLabel.MDL_PICKING_FAMILY, f.getName(), id);
				md.setValueInt(MDLabel.MDL_PICKING_COLOR, f.getColor().getRGB(), id);
				md.setValueInt(MDLabel.MDL_PICKING_PARTICLE_SIZE, f.getSize(), id);
				md.setValueString(MDLabel.MDL_PICKING_FAMILY_STATE, f.getStep().toString(), id);
			}
			md.write(file);
		}
		catch (Exception e)
		{
			getLogger().log(Level.SEVERE, e.getMessage(), e);
			throw new IllegalArgumentException(e.getMessage());
		}
	}

	public void loadFamilies()
	{
		families.clear();
		String file = familiesfile;
		if (!new File(file).exists())
		{
			families.add(Family.getDefaultFamily());
			return;
		}

		Family family;
		int rgb, size;
		FamilyState state;
		String name;
		try
		{
			MetaData md = new MetaData(file);
			long[] ids = md.findObjects();
			for (long id : ids)
			{
				name = md.getValueString(MDLabel.MDL_PICKING_FAMILY, id);
				rgb = md.getValueInt(MDLabel.MDL_PICKING_COLOR, id);
				size = md.getValueInt(MDLabel.MDL_PICKING_PARTICLE_SIZE, id);
				state = FamilyState.valueOf(md.getValueString(MDLabel.MDL_PICKING_FAMILY_STATE, id));
				state = validateState(state);
				if (state == FamilyState.Supervised && this instanceof SupervisedParticlePicker)
					if (!new File(((SupervisedParticlePicker) this).getTrainingFile(name)).exists())
						throw new IllegalArgumentException(String.format("Training file does not exist. Family cannot be in %s mode", state));
				family = new Family(name, new Color(rgb), size, state, this);
				families.add(family);
			}
			if (families.size() == 0)
				throw new IllegalArgumentException(String.format("No families specified on %s", file));
		}
		catch (Exception e)
		{
			getLogger().log(Level.SEVERE, e.getMessage(), e);
			throw new IllegalArgumentException(e.getMessage());
		}
	}

	public FamilyState validateState(FamilyState state)
	{

		if (mode == FamilyState.Review && state != FamilyState.Review)
		{
			setChanged(true);
			return FamilyState.Review;
		}
		if (mode == FamilyState.Manual && !(state == FamilyState.Manual || state == FamilyState.Available))
			throw new IllegalArgumentException(String.format("Can not use %s mode on this data", mode));
		if (mode == FamilyState.Supervised && state == FamilyState.Review)
			throw new IllegalArgumentException(String.format("Can not use %s mode on this data", mode));
		return state;

	}

	public Family getFamily(String name)
	{
		for (Family f : getFamilies())
			if (f.getName().equalsIgnoreCase(name))
				return f;
		return null;
	}

	public boolean existsFamilyName(String name)
	{
		return getFamily(name) != null;
	}

	protected boolean containsBlock(String file, String block)
	{
		try
		{
			return Arrays.asList(MetaData.getBlocksInMetaDataFile(file)).contains(block);
		}
		catch (Exception e)
		{
			getLogger().log(Level.SEVERE, e.getMessage(), e);
			throw new IllegalArgumentException(e);
		}
	}

	public void removeFamily(Family family)
	{
		if (getManualParticlesNumber(family) > 0)// perhaps I have to check
													// automatic particles
			throw new IllegalArgumentException(XmippMessage.getAssociatedDataMsg("family"));
		if (families.size() == 1)
			throw new IllegalArgumentException(XmippMessage.getIllegalDeleteMsg("family"));
		families.remove(family);
	}

	public void saveData()
	{
		persistFilters();
		persistFamilies();

	}

	public abstract int getManualParticlesNumber(Family f);

	public void persistFilters()
	{
		long id;
		String file = macrosfile;
		if (filters.isEmpty())
		{
			new File(file).delete();
			return;
		}
		String options;
		try
		{
			MetaData md = new MetaData();
			for (Filter f : filters)
			{
				id = md.addObject();
				md.setValueString(MDLabel.MDL_ASSOCIATED_IMAGE1, f.getCommand().replace(' ', '_'), id);
				options = (f.getOptions() == null || f.getOptions().equals("")) ? "NULL" : f.getOptions().replace(' ', '_');
				md.setValueString(MDLabel.MDL_ASSOCIATED_IMAGE2, options, id);
			}
			md.write(file);
		}
		catch (Exception e)
		{
			getLogger().log(Level.SEVERE, e.getMessage(), e);
			throw new IllegalArgumentException(e.getMessage());
		}

	}

	public void loadFilters()
	{
		filters.clear();
		String file = macrosfile;
		if (!new File(file).exists())
			return;

		String command, options;
		try
		{
			MetaData md = new MetaData(file);
			long[] ids = md.findObjects();
			for (long id : ids)
			{
				command = md.getValueString(MDLabel.MDL_ASSOCIATED_IMAGE1, id).replace('_', ' ');
				options = md.getValueString(MDLabel.MDL_ASSOCIATED_IMAGE2, id).replace(' ', '_');
				if (options.equals("NULL"))
					options = "";
				filters.add(new Filter(command, options));

			}
			System.out.println(filters.size());
		}
		catch (Exception e)
		{
			getLogger().log(Level.SEVERE, e.getMessage(), e);
			throw new IllegalArgumentException(e.getMessage());
		}
	}

	void removeFilter(String filter)
	{
		for (Filter f : filters)
			if (f.getCommand().equals(filter))
			{
				filters.remove(f);
				setChanged(true);
				break;

			}

	}

	public boolean isFilterSelected(String filter)
	{
		for (Filter f : filters)
			if (f.getCommand().equals(filter))
				return true;
		return false;
	}

	public abstract void exportParticles(String absolutePath);

	public abstract void importParticlesFromXmipp30Folder(String absolutePath);

	public abstract void importParticlesFromXmipp24Folder(String path);

	public abstract void importParticlesFromEmanFolder(String path);

	public void runXmippProgram(String program, String args)
	{
		try
		{
			Program.runByName(program, args);
		}
		catch (Exception e)
		{
			TrainingPicker.getLogger().log(Level.SEVERE, e.getMessage(), e);
			throw new IllegalArgumentException(e);
		}
	}

}
