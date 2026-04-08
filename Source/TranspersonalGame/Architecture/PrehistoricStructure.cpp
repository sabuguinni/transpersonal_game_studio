#include "PrehistoricStructure.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

APrehistoricStructure::APrehistoricStructure()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    // Create exterior mesh component
    ExteriorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExteriorMesh"));
    ExteriorMesh->SetupAttachment(RootComponent);
    ExteriorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    ExteriorMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);

    // Create interior bounds for player detection
    InteriorBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("InteriorBounds"));
    InteriorBounds->SetupAttachment(RootComponent);
    InteriorBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteriorBounds->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    InteriorBounds->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    InteriorBounds->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Create ambient audio component
    AmbientAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudio->SetupAttachment(RootComponent);
    AmbientAudio->bAutoActivate = false;

    // Initialize default values
    bPlayerHasVisited = false;
    TimeOfLastVisit = 0.0f;
    bIsCurrentlyOccupiedByWildlife = false;
    bShowBloodStains = false;
    bShowToolMarks = true;
    bShowFirePit = false;
    bShowFoodRemains = false;
    bIsInitialized = false;
    bStoryTextCached = false;
    CreationTime = 0.0f;

    // Bind overlap events
    InteriorBounds->OnComponentBeginOverlap.AddDynamic(this, &APrehistoricStructure::OnInteriorEntered);
    InteriorBounds->OnComponentEndOverlap.AddDynamic(this, &APrehistoricStructure::OnInteriorExited);
}

void APrehistoricStructure::BeginPlay()
{
    Super::BeginPlay();
    
    CreationTime = GetWorld()->GetTimeSeconds();
    
    if (StructureData)
    {
        InitializeFromData(StructureData);
    }
}

void APrehistoricStructure::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update dynamic elements based on time and conditions
    if (bIsInitialized)
    {
        // Simulate natural decay over time
        float TimeSinceCreation = GetWorld()->GetTimeSeconds() - CreationTime;
        
        // Update wildlife occupation probability
        if (!bPlayerHasVisited && FMath::RandRange(0.0f, 1.0f) < 0.001f) // Small chance per frame
        {
            bIsCurrentlyOccupiedByWildlife = FMath::RandBool();
        }
    }
}

void APrehistoricStructure::InitializeFromData(UPrehistoricStructureData* Data)
{
    if (!Data)
    {
        UE_LOG(LogTemp, Warning, TEXT("PrehistoricStructure: No structure data provided"));
        return;
    }

    StructureData = Data;

    // Set exterior mesh
    if (Data->ExteriorMesh.LoadSynchronous())
    {
        ExteriorMesh->SetStaticMesh(Data->ExteriorMesh.LoadSynchronous());
    }

    // Configure interior bounds based on structure dimensions
    InteriorBounds->SetBoxExtent(Data->Dimensions * 0.5f);

    // Generate interior layout
    GenerateInteriorLayout();

    // Apply weathering effects based on condition
    ApplyWeatheringEffects();

    // Setup ambient audio
    SetupAmbientAudio();

    // Set storytelling elements based on narrative
    const FInteriorNarrative& Narrative = Data->InteriorStory;
    
    // Determine visible story elements
    if (Narrative.bShowsSignsOfStruggle)
    {
        bShowBloodStains = true;
        VisibleClues.Add(TEXT("Dark stains on the floor suggest violence"));
        VisibleClues.Add(TEXT("Scattered belongings indicate a hasty departure"));
    }
    else
    {
        bShowFirePit = true;
        VisibleClues.Add(TEXT("Cold ashes in a carefully built fire pit"));
        VisibleClues.Add(TEXT("Neatly arranged sleeping area"));
    }

    // Add tool marks based on structure type
    if (Data->StructureType == EPrehistoricStructureType::Workshop)
    {
        bShowToolMarks = true;
        VisibleClues.Add(TEXT("Stone surfaces show signs of tool sharpening"));
        VisibleClues.Add(TEXT("Bone fragments scattered around work area"));
    }

    // Add food remains for certain structure types
    if (Data->StructureType == EPrehistoricStructureType::Hut || 
        Data->StructureType == EPrehistoricStructureType::Cave)
    {
        if (Narrative.YearsSinceAbandonment < 5.0f)
        {
            bShowFoodRemains = true;
            VisibleClues.Add(TEXT("Dried meat hanging from wooden frame"));
        }
    }

    bIsInitialized = true;
    bStoryTextCached = false; // Force story regeneration

    UE_LOG(LogTemp, Log, TEXT("PrehistoricStructure initialized: %s"), *Data->StructureName);
}

void APrehistoricStructure::GenerateInteriorLayout()
{
    if (!StructureData)
        return;

    // Clear existing interior props
    for (UStaticMeshComponent* Prop : InteriorProps)
    {
        if (Prop)
        {
            Prop->DestroyComponent();
        }
    }
    InteriorProps.Empty();

    // Generate props based on structure type and narrative
    const FInteriorNarrative& Narrative = StructureData->InteriorStory;
    
    // Basic furniture/props for all inhabited structures
    if (StructureData->StructureType != EPrehistoricStructureType::Ruins)
    {
        // Sleeping area
        FTransform SleepingTransform;
        SleepingTransform.SetLocation(FVector(-100.0f, 0.0f, 0.0f));
        // SpawnInteriorProp would be called here with sleeping fur mesh
        
        // Storage area
        FTransform StorageTransform;
        StorageTransform.SetLocation(FVector(100.0f, 100.0f, 0.0f));
        // SpawnInteriorProp would be called here with storage basket mesh
    }

    // Specific props based on structure type
    switch (StructureData->StructureType)
    {
        case EPrehistoricStructureType::Workshop:
            {
                // Tool making area
                FTransform WorkbenchTransform;
                WorkbenchTransform.SetLocation(FVector(0.0f, -100.0f, 0.0f));
                // SpawnInteriorProp for stone anvil
                break;
            }
        case EPrehistoricStructureType::Ceremonial:
            {
                // Central altar or fire pit
                FTransform AltarTransform;
                AltarTransform.SetLocation(FVector::ZeroVector);
                // SpawnInteriorProp for stone altar
                break;
            }
        case EPrehistoricStructureType::Storage:
            {
                // Multiple storage containers
                for (int32 i = 0; i < 3; i++)
                {
                    FTransform ContainerTransform;
                    ContainerTransform.SetLocation(FVector(i * 80.0f - 80.0f, 0.0f, 0.0f));
                    // SpawnInteriorProp for storage containers
                }
                break;
            }
    }

    UE_LOG(LogTemp, Log, TEXT("Interior layout generated for %s"), 
           *UEnum::GetValueAsString(StructureData->StructureType));
}

void APrehistoricStructure::SpawnInteriorProp(UStaticMesh* PropMesh, const FTransform& Transform, const FString& StoryContext)
{
    if (!PropMesh)
        return;

    // Create new static mesh component for the prop
    FString PropName = FString::Printf(TEXT("InteriorProp_%d"), InteriorProps.Num());
    UStaticMeshComponent* PropComponent = CreateDefaultSubobject<UStaticMeshComponent>(*PropName);
    
    if (PropComponent)
    {
        PropComponent->SetupAttachment(RootComponent);
        PropComponent->SetStaticMesh(PropMesh);
        PropComponent->SetWorldTransform(Transform);
        PropComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        
        InteriorProps.Add(PropComponent);
        
        // Add story context to visible clues if provided
        if (!StoryContext.IsEmpty())
        {
            VisibleClues.Add(StoryContext);
        }

        UE_LOG(LogTemp, Log, TEXT("Spawned interior prop: %s"), *StoryContext);
    }
}

void APrehistoricStructure::ApplyWeatheringEffects()
{
    if (!StructureData)
        return;

    // Apply material modifications based on structure condition
    UMaterialInterface* BaseMaterial = ExteriorMesh->GetMaterial(0);
    
    if (BaseMaterial)
    {
        // Create dynamic material instance to modify weathering parameters
        UMaterialInstanceDynamic* DynamicMaterial = ExteriorMesh->CreateAndSetMaterialInstanceDynamic(0);
        
        if (DynamicMaterial)
        {
            float WeatheringAmount = 0.0f;
            
            switch (StructureData->Condition)
            {
                case EStructureCondition::Pristine:
                    WeatheringAmount = 0.1f;
                    break;
                case EStructureCondition::Weathered:
                    WeatheringAmount = 0.4f;
                    break;
                case EStructureCondition::Damaged:
                    WeatheringAmount = 0.7f;
                    break;
                case EStructureCondition::Ruined:
                    WeatheringAmount = 0.9f;
                    break;
                case EStructureCondition::Collapsed:
                    WeatheringAmount = 1.0f;
                    break;
                case EStructureCondition::Overgrown:
                    WeatheringAmount = 0.8f;
                    DynamicMaterial->SetScalarParameterValue(TEXT("VegetationCoverage"), 0.6f);
                    break;
            }
            
            DynamicMaterial->SetScalarParameterValue(TEXT("WeatheringAmount"), WeatheringAmount);
            DynamicMaterial->SetScalarParameterValue(TEXT("AgeInYears"), StructureData->InteriorStory.YearsSinceAbandonment);
        }
    }
}

void APrehistoricStructure::SetupAmbientAudio()
{
    if (!AmbientAudio)
        return;

    // Set ambient audio based on structure type and condition
    if (StructureData)
    {
        // Different ambient sounds for different structure types
        switch (StructureData->StructureType)
        {
            case EPrehistoricStructureType::Cave:
                // Water dripping, echo effects
                AmbientAudio->SetVolumeMultiplier(0.3f);
                break;
            case EPrehistoricStructureType::Ruins:
                // Wind through broken structures
                AmbientAudio->SetVolumeMultiplier(0.5f);
                break;
            default:
                // Subtle creaking, settling sounds
                AmbientAudio->SetVolumeMultiplier(0.2f);
                break;
        }

        // Modify audio based on wildlife occupation
        if (bIsCurrentlyOccupiedByWildlife)
        {
            AmbientAudio->SetVolumeMultiplier(0.1f); // Quieter when animals are present
        }
    }
}

FString APrehistoricStructure::GetStructureStory() const
{
    if (!StructureData)
        return TEXT("An ancient structure of unknown origin.");

    if (bStoryTextCached)
        return CachedStoryText;

    const FInteriorNarrative& Narrative = StructureData->InteriorStory;
    
    FString StoryText;
    StoryText += FString::Printf(TEXT("This %s was once home to %s. "), 
                                *UEnum::GetValueAsString(StructureData->StructureType),
                                *Narrative.FormerInhabitant);
    
    StoryText += FString::Printf(TEXT("%s "), *Narrative.FateOfInhabitant);
    
    if (Narrative.YearsSinceAbandonment < 1.0f)
    {
        StoryText += TEXT("The abandonment seems recent - personal belongings remain untouched. ");
    }
    else if (Narrative.YearsSinceAbandonment < 5.0f)
    {
        StoryText += TEXT("Nature has begun to reclaim this place, but traces of life remain. ");
    }
    else
    {
        StoryText += TEXT("Time has weathered this place, leaving only echoes of its former inhabitants. ");
    }

    if (bIsCurrentlyOccupiedByWildlife)
    {
        StoryText += TEXT("Small animals have made this their shelter now. ");
    }

    // Cache the story text
    CachedStoryText = StoryText;
    bStoryTextCached = true;

    return StoryText;
}

TArray<FString> APrehistoricStructure::GetVisibleClues() const
{
    return VisibleClues;
}

void APrehistoricStructure::RevealHiddenClue(const FString& ClueName)
{
    if (!VisibleClues.Contains(ClueName))
    {
        VisibleClues.Add(ClueName);
        UE_LOG(LogTemp, Log, TEXT("Revealed hidden clue: %s"), *ClueName);
    }
}

bool APrehistoricStructure::CanProvideShel() const
{
    if (!StructureData)
        return false;

    // Check if structure is intact enough to provide shelter
    return StructureData->bCanProvideShel && 
           StructureData->Condition != EStructureCondition::Collapsed &&
           !bIsCurrentlyOccupiedByWildlife;
}

float APrehistoricStructure::GetSafetyRating() const
{
    if (!StructureData)
        return 0.0f;

    float SafetyModifier = 1.0f;

    // Reduce safety if occupied by wildlife
    if (bIsCurrentlyOccupiedByWildlife)
        SafetyModifier *= 0.3f;

    // Reduce safety based on structural condition
    switch (StructureData->Condition)
    {
        case EStructureCondition::Collapsed:
            SafetyModifier *= 0.1f;
            break;
        case EStructureCondition::Ruined:
            SafetyModifier *= 0.4f;
            break;
        case EStructureCondition::Damaged:
            SafetyModifier *= 0.7f;
            break;
        default:
            break;
    }

    return StructureData->SafetyRating * SafetyModifier;
}

bool APrehistoricStructure::ContainsUsableResources() const
{
    if (!StructureData)
        return false;

    return StructureData->bContainsResources && !bPlayerHasVisited;
}

void APrehistoricStructure::SetWildlifeOccupied(bool bOccupied)
{
    bIsCurrentlyOccupiedByWildlife = bOccupied;
    
    // Update ambient audio
    SetupAmbientAudio();
    
    // Invalidate cached story
    bStoryTextCached = false;
}

void APrehistoricStructure::OnInteriorEntered(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                                            bool bFromSweep, const FHitResult& SweepResult)
{
    // Check if it's the player
    if (OtherActor && OtherActor->IsA<APawn>())
    {
        APlayerController* PC = Cast<APlayerController>(Cast<APawn>(OtherActor)->GetController());
        if (PC)
        {
            bPlayerHasVisited = true;
            TimeOfLastVisit = GetWorld()->GetTimeSeconds();
            
            // Scare away wildlife if present
            if (bIsCurrentlyOccupiedByWildlife)
            {
                SetWildlifeOccupied(false);
            }

            // Activate ambient audio
            if (AmbientAudio)
            {
                AmbientAudio->Activate();
            }

            // Trigger Blueprint event
            OnPlayerEntered();

            UE_LOG(LogTemp, Log, TEXT("Player entered %s"), 
                   StructureData ? *StructureData->StructureName : TEXT("Unknown Structure"));
        }
    }
}

void APrehistoricStructure::OnInteriorExited(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // Check if it's the player
    if (OtherActor && OtherActor->IsA<APawn>())
    {
        APlayerController* PC = Cast<APlayerController>(Cast<APawn>(OtherActor)->GetController());
        if (PC)
        {
            // Deactivate ambient audio
            if (AmbientAudio)
            {
                AmbientAudio->Deactivate();
            }

            // Trigger Blueprint event
            OnPlayerExited();

            UE_LOG(LogTemp, Log, TEXT("Player exited %s"), 
                   StructureData ? *StructureData->StructureName : TEXT("Unknown Structure"));
        }
    }
}