// Plant Tycoon Game Logic will go here
console.log("Script loaded");

document.addEventListener('DOMContentLoaded', () => {
    // Initial game setup can go here
    console.log("Game starting...");

    // Example: Initialize some garden plots
    const plantPlotsContainer = document.getElementById('plant-plots');
    const numPlots = 6; // Create 6 plots for now

    for (let i = 0; i < numPlots; i++) {
        const plot = document.createElement('div');
        plot.classList.add('plant-plot', 'empty');
        plot.dataset.plotId = i;
        plot.textContent = 'Empty Plot';
        plot.addEventListener('click', () => handlePlotClick(plot));
        plantPlotsContainer.appendChild(plot);
    }

    // Example: Buy seed button functionality
    const buySeedButton = document.getElementById('buy-seed-btn');
    buySeedButton.addEventListener('click', buySeed);

    updateStats();
});

let money = 100;
let seedsOwned = {
    'common': 0, // Example seed type
    // Add more seed types as needed
};

let garden = {}; // To store plant data, e.g., { plotId: { species: 'Rose', growth: 0, health: 100 }}
let nursery = []; // To store plants ready for sale, e.g., [{ species: 'Rose', value: 50, id: 'uniqueId1'}]

function updateStats() {
    document.getElementById('money').textContent = money;
    const totalSeeds = Object.values(seedsOwned).reduce((sum, count) => sum + count, 0);
    document.getElementById('seeds-owned').textContent = totalSeeds;
}

function buySeed() {
    const seedCost = 10;
    if (money >= seedCost) {
        money -= seedCost;
        seedsOwned['common'] = (seedsOwned['common'] || 0) + 1; // Buy a 'common' seed
        console.log("Bought a common seed.");
        updateStats();
    } else {
        console.log("Not enough money to buy a seed.");
        alert("Not enough money!");
    }
}

function handlePlotClick(plotElement) {
    const plotId = plotElement.dataset.plotId;
    console.log(`Clicked on plot ${plotId}`);

    if (plotElement.classList.contains('empty')) {
        // Try to plant a seed if available
        if (seedsOwned['common'] > 0) {
            plantSeed(plotId, 'common', plotElement);
        } else {
            console.log("No common seeds to plant.");
            alert("You don't have any common seeds! Buy some from the shop.");
        }
    } else if (garden[plotId]) {
        // Interact with existing plant (e.g., water, harvest)
        interactWithPlant(plotId, plotElement);
    }
}

function plantSeed(plotId, seedType, plotElement) {
    if (seedsOwned[seedType] > 0) {
        seedsOwned[seedType]--;
        garden[plotId] = {
            species: seedType, // For now, species is same as seedType
            growth: 0, // 0-100%
            health: 100, // 0-100%
            plantedTime: Date.now(),
            isMature: false
        };
        plotElement.classList.remove('empty');
        plotElement.classList.add('planted');
        // Add a visual representation of the plant
        const plantDiv = document.createElement('div');
        plantDiv.classList.add('plant');
        plantDiv.innerHTML = `<span class="species">${seedType}</span><span class="growth">0%</span>`;
        plotElement.innerHTML = ''; // Clear 'Empty Plot' text
        plotElement.appendChild(plantDiv);

        console.log(`${seedType} seed planted in plot ${plotId}`);
        updateStats();
        // Start growth process
        growPlant(plotId, plotElement);
    } else {
        console.log(`No ${seedType} seeds available.`);
    }
}

function growPlant(plotId, plotElement) {
    const plantData = garden[plotId];
    if (!plantData || plantData.isMature) return;

    const growthInterval = setInterval(() => {
        if (plantData.growth < 100) {
            plantData.growth += 10; // Grow by 10% each interval for demo
            const plantDiv = plotElement.querySelector('.plant .growth');
            if (plantDiv) {
                plantDiv.textContent = `${plantData.growth}%`;
            }
            console.log(`Plant in plot ${plotId} grew to ${plantData.growth}%`);
        } else {
            plantData.isMature = true;
            clearInterval(growthInterval);
            const speciesSpan = plotElement.querySelector('.plant .species');
            if (speciesSpan) speciesSpan.textContent += ' (Mature)';
            console.log(`Plant in plot ${plotId} is mature.`);
            // Add option to move to nursery or harvest
            plotElement.title = 'Click to move to nursery';
        }
    }, 2000); // Grow every 2 seconds for demo
}

function interactWithPlant(plotId, plotElement) {
    const plantData = garden[plotId];
    if (plantData && plantData.isMature) {
        // Move to nursery
        moveToNursery(plotId, plotElement);
    } else if (plantData) {
        // Could add watering logic here, etc.
        console.log(`Interacting with plant: ${plantData.species}, Growth: ${plantData.growth}%`);
        alert(`Plant: ${plantData.species}\nGrowth: ${plantData.growth}%\nHealth: ${plantData.health}%`);
    }
}

function moveToNursery(plotId, plotElement) {
    const plantData = garden[plotId];
    if (!plantData || !plantData.isMature) return;

    const plantValue = calculatePlantValue(plantData);
    const nurseryPlant = {
        id: `plant-${Date.now()}-${Math.random().toString(16).slice(2)}`,
        species: plantData.species,
        value: plantValue,
        // Potentially add more details like genetics, age, etc.
    };
    nursery.push(nurseryPlant);

    // Clear the plot
    delete garden[plotId];
    plotElement.innerHTML = 'Empty Plot';
    plotElement.classList.remove('planted');
    plotElement.classList.add('empty');
    plotElement.title = '';

    console.log(`${plantData.species} moved to nursery. Value: ${plantValue}`);
    updateNurseryDisplay();
    updateStats();
}

function calculatePlantValue(plantData) {
    // Simple value calculation for now
    let baseValue = 20; // Base value for a common mature plant
    if (plantData.species === 'common') baseValue = 20;
    // Could add bonuses for health, specific species, etc.
    return baseValue + Math.floor(plantData.health / 10); // e.g. +10 for 100 health
}

function updateNurseryDisplay() {
    const nurseryContainer = document.getElementById('nursery-plants');
    nurseryContainer.innerHTML = ''; // Clear existing nursery items

    nursery.forEach(plant => {
        const item = document.createElement('div');
        item.classList.add('nursery-plant-item');
        item.innerHTML = `
            <span>${plant.species} (Value: ${plant.value})</span>
            <button class="sell-btn" data-plant-id="${plant.id}">Sell</button>
        `;
        nurseryContainer.appendChild(item);
    });

    // Add event listeners to new sell buttons
    document.querySelectorAll('.sell-btn').forEach(button => {
        button.addEventListener('click', (e) => {
            const plantIdToSell = e.target.dataset.plantId;
            sellPlantFromNursery(plantIdToSell);
        });
    });
}

function sellPlantFromNursery(plantId) {
    const plantIndex = nursery.findIndex(p => p.id === plantId);
    if (plantIndex > -1) {
        const plantToSell = nursery[plantIndex];
        money += plantToSell.value;
        nursery.splice(plantIndex, 1); // Remove plant from nursery

        console.log(`Sold ${plantToSell.species} for ${plantToSell.value}. Current money: ${money}`);
        updateStats();
        updateNurseryDisplay();
    } else {
        console.error(`Could not find plant with ID ${plantId} in nursery to sell.`);
    }
}

// TODO:
// - More complex plant genetics and cross-breeding
// - Different seed types and plant species
// - Plant health affected by watering, pests, etc.
// - Soil types, fertilizers
// - Researching new plants/upgrades
// - Saving and loading game state (localStorage)
// - More detailed UI and animations
