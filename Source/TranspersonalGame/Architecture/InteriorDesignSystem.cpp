#include "InteriorDesignSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "DrawDebugHelpers.h"

AInteriorDesignSystem::AInteriorDesignSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Set default values
    PropDensity = 0.7f; // Moderate prop density
    StorytellingIntensity = 0.8f; // High storytelling focus

    // Initialize zone layouts
    ZoneLayouts.Empty();
}

void AInteriorDesignSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("InteriorDesignSystem: Initializing prehistoric interior design system"));
    
    InitializePropLibrary();
}

void AInteriorDesignSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AInteriorDesignSystem::DesignInterior(AActor* Building, const FVector& InteriorBounds, const TArray<FString>& StoryElements)
{
    if (!Building || !GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("InteriorDesignSystem: Invalid building or world for interior design"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("InteriorDesignSystem: Designing interior for building at %s"), 
           *Building->GetActorLocation().ToString());

    FVector BuildingLocation = Building->GetActorLocation();
    
    // Clear existing zone layouts
    ZoneLayouts.Empty();

    // Determine interior layout based on building size and story elements
    float InteriorArea = InteriorBounds.X * InteriorBounds.Y;
    
    if (InteriorArea > 400000.0f) // Large building (20m x 20m)
    {
        // Multi-zone layout
        CreateZoneLayout(EInteriorZone::Entrance, BuildingLocation + FVector(0, -InteriorBounds.Y * 0.3f, 0), 
                        FVector(InteriorBounds.X * 0.8f, InteriorBounds.Y * 0.2f, InteriorBounds.Z), 
                        TEXT("Well-worn entrance with scattered footprints"));
        
        CreateZoneLayout(EInteriorZone::LivingArea, BuildingLocation, 
                        FVector(InteriorBounds.X * 0.6f, InteriorBounds.Y * 0.4f, InteriorBounds.Z), 
                        TEXT("Central gathering space with fire pit"));
        
        CreateZoneLayout(EInteriorZone::SleepingArea, BuildingLocation + FVector(InteriorBounds.X * 0.25f, InteriorBounds.Y * 0.25f, 0), 
                        FVector(InteriorBounds.X * 0.3f, InteriorBounds.Y * 0.3f, InteriorBounds.Z), 
                        TEXT("Private sleeping area with personal belongings"));
        
        CreateZoneLayout(EInteriorZone::StorageArea, BuildingLocation + FVector(-InteriorBounds.X * 0.25f, InteriorBounds.Y * 0.25f, 0), 
                        FVector(InteriorBounds.X * 0.3f, InteriorBounds.Y * 0.3f, InteriorBounds.Z), 
                        TEXT("Storage area with preserved foods and tools"));
    }
    else if (InteriorArea > 100000.0f) // Medium building (10m x 10m)
    {
        // Dual-zone layout
        CreateZoneLayout(EInteriorZone::LivingArea, BuildingLocation + FVector(0, -InteriorBounds.Y * 0.2f, 0), 
                        FVector(InteriorBounds.X * 0.8f, InteriorBounds.Y * 0.6f, InteriorBounds.Z), 
                        TEXT("Combined living and working space"));
        
        CreateZoneLayout(EInteriorZone::SleepingArea, BuildingLocation + FVector(0, InteriorBounds.Y * 0.2f, 0), 
                        FVector(InteriorBounds.X * 0.8f, InteriorBounds.Y * 0.4f, InteriorBounds.Z), 
                        TEXT("Sleeping area with family possessions"));
    }
    else
    {
        // Single-zone layout
        CreateZoneLayout(EInteriorZone::LivingArea, BuildingLocation, 
                        FVector(InteriorBounds.X * 0.9f, InteriorBounds.Y * 0.9f, InteriorBounds.Z), 
                        TEXT("Single-room dwelling with all life activities"));
    }

    // Populate each zone with appropriate props
    for (FInteriorZoneLayout& Zone : ZoneLayouts)
    {
        PopulateZoneWithProps(Zone);
    }

    // Add story-specific elements
    for (const FString& StoryElement : StoryElements)
    {
        AddStorytellingElements(BuildingLocation, StoryElement);
    }

    // Apply lived-in details
    ApplyLivedInDetails(Building, StorytellingIntensity);

    UE_LOG(LogTemp, Log, TEXT("InteriorDesignSystem: Completed interior design with %d zones"), ZoneLayouts.Num());
}

void AInteriorDesignSystem::CreateZoneLayout(EInteriorZone ZoneType, const FVector& ZoneCenter, const FVector& ZoneBounds, const FString& Story)
{
    FInteriorZoneLayout NewZone;
    NewZone.ZoneType = ZoneType;
    NewZone.ZoneCenter = ZoneCenter;
    NewZone.ZoneBounds = ZoneBounds;
    NewZone.ZoneStory = Story;
    
    // Set usage intensity based on zone type
    switch (ZoneType)
    {
        case EInteriorZone::LivingArea:
            NewZone.UsageIntensity = 0.9f; // High usage
            break;
        case EInteriorZone::SleepingArea:
            NewZone.UsageIntensity = 0.7f; // Moderate usage
            break;
        case EInteriorZone::StorageArea:
            NewZone.UsageIntensity = 0.4f; // Low usage
            break;
        case EInteriorZone::WorkArea:
            NewZone.UsageIntensity = 0.8f; // High usage
            break;
        default:
            NewZone.UsageIntensity = 0.5f; // Default
            break;
    }

    ZoneLayouts.Add(NewZone);
    
    UE_LOG(LogTemp, Log, TEXT("InteriorDesignSystem: Created %s zone at %s"), 
           *UEnum::GetValueAsString(ZoneType), *ZoneCenter.ToString());
}

void AInteriorDesignSystem::PopulateZoneWithProps(FInteriorZoneLayout& Zone)
{
    if (!GetWorld())
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("InteriorDesignSystem: Populating %s zone with props"), 
           *UEnum::GetValueAsString(Zone.ZoneType));

    // Determine number of props based on zone size and density
    float ZoneArea = Zone.ZoneBounds.X * Zone.ZoneBounds.Y;
    int32 MaxProps = FMath::RoundToInt((ZoneArea / 10000.0f) * PropDensity * Zone.UsageIntensity); // 1 prop per square meter at full density

    // Find suitable prop placement locations
    TArray<FVector> PropLocations = FindPropPlacementLocations(Zone, MaxProps);

    // Create zone-specific props
    switch (Zone.ZoneType)
    {
        case EInteriorZone::LivingArea:
            CreateFirePitArea(Zone.ZoneCenter);
            // Add seating, gathering props
            break;
            
        case EInteriorZone::SleepingArea:
            CreateSleepingArea(Zone.ZoneCenter, FMath::RandRange(1, 4));
            break;
            
        case EInteriorZone::StorageArea:
            {
                TArray<FString> StoredItems = {TEXT("Dried meat"), TEXT("Stone tools"), TEXT("Animal hides"), TEXT("Seeds")};
                CreateStorageArea(Zone.ZoneCenter, StoredItems);
            }
            break;
            
        case EInteriorZone::WorkArea:
            CreateWorkArea(Zone.ZoneCenter, TEXT("Stone tool crafting"));
            break;
            
        default:
            break;
    }

    // Add random props at found locations
    for (int32 i = 0; i < FMath::Min(PropLocations.Num(), MaxProps); i++)
    {
        FVector PropLocation = PropLocations[i];
        
        // Select appropriate prop type for zone
        EInteriorPropType PropType = EInteriorPropType::Furniture;
        switch (Zone.ZoneType)
        {
            case EInteriorZone::LivingArea:
                PropType = (FMath::RandBool()) ? EInteriorPropType::Furniture : EInteriorPropType::PersonalItems;
                break;
            case EInteriorZone::SleepingArea:
                PropType = (FMath::RandBool()) ? EInteriorPropType::PersonalItems : EInteriorPropType::Containers;
                break;
            case EInteriorZone::StorageArea:
                PropType = EInteriorPropType::Containers;
                break;
            case EInteriorZone::WorkArea:
                PropType = (FMath::RandBool()) ? EInteriorPropType::Tools : EInteriorPropType::CraftingMaterials;
                break;
            default:
                PropType = EInteriorPropType::Decorations;
                break;
        }

        // Create prop
        FInteriorProp NewProp;
        NewProp.PropType = PropType;
        NewProp.RelativeLocation = PropLocation - Zone.ZoneCenter;
        NewProp.RelativeRotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);
        NewProp.WearLevel = FMath::RandRange(0.2f, 0.8f);
        NewProp.StoryDescription = FString::Printf(TEXT("A %s showing signs of regular use"), 
                                                  *UEnum::GetValueAsString(PropType));

        // Get appropriate mesh
        NewProp.Mesh = SelectAppropriateProps(PropType, Zone.ZoneStory);

        Zone.Props.Add(NewProp);

        // Spawn the actual prop actor
        if (NewProp.Mesh)
        {
            AStaticMeshActor* PropActor = GetWorld()->SpawnActor<AStaticMeshActor>(PropLocation, NewProp.RelativeRotation);
            if (PropActor)
            {
                PropActor->GetStaticMeshComponent()->SetStaticMesh(NewProp.Mesh);
                PropActor->SetActorScale3D(NewProp.Scale);
                
                // Add tags for identification
                PropActor->Tags.Add(FName("InteriorProp"));
                PropActor->Tags.Add(FName(*UEnum::GetValueAsString(PropType)));
                PropActor->Tags.Add(FName(*UEnum::GetValueAsString(Zone.ZoneType)));

                // Apply wear and tear
                AddWearAndTearEffects(PropActor, NewProp.WearLevel);
            }
        }
    }
}

void AInteriorDesignSystem::AddStorytellingElements(const FVector& Location, const FString& StoryElement)
{
    if (!GetWorld())
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("InteriorDesignSystem: Adding storytelling element: %s at %s"), 
           *StoryElement, *Location.ToString());

    // Parse story element and create appropriate visual representation
    if (StoryElement.Contains(TEXT("fire")) || StoryElement.Contains(TEXT("burnt")))
    {
        CreateFirePitArea(Location);
    }
    else if (StoryElement.Contains(TEXT("sleeping")) || StoryElement.Contains(TEXT("hide")))
    {
        CreateSleepingArea(Location, 1);
    }
    else if (StoryElement.Contains(TEXT("tools")) || StoryElement.Contains(TEXT("stone")))
    {
        CreateWorkArea(Location, TEXT("Tool maintenance"));
    }

    // Add environmental clue
    AddEnvironmentalClues(Location, StoryElement);
}

void AInteriorDesignSystem::ApplyLivedInDetails(AActor* Building, float IntensityLevel)
{
    if (!Building)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("InteriorDesignSystem: Applying lived-in details with intensity %.2f"), IntensityLevel);

    FVector BuildingLocation = Building->GetActorLocation();

    // Add wear patterns, dirt accumulation, personal touches based on intensity
    int32 NumDetails = FMath::RoundToInt(IntensityLevel * 10.0f);

    for (int32 i = 0; i < NumDetails; i++)
    {
        FVector DetailLocation = BuildingLocation + FVector(
            FMath::RandRange(-200.0f, 200.0f),
            FMath::RandRange(-200.0f, 200.0f),
            0.0f
        );

        // Add random personal touches
        TArray<FString> PersonalityTraits = {
            TEXT("Meticulous organizer"),
            TEXT("Skilled craftsperson"),
            TEXT("Spiritual practitioner"),
            TEXT("Practical survivor"),
            TEXT("Family caretaker")
        };

        FString RandomTrait = PersonalityTraits[FMath::RandRange(0, PersonalityTraits.Num() - 1)];
        CreatePersonalTouches(DetailLocation, RandomTrait);
    }
}

void AInteriorDesignSystem::CreateAbandonmentEffects(AActor* Building, float AbandonmentLevel)
{
    if (!Building || AbandonmentLevel <= 0.0f)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("InteriorDesignSystem: Creating abandonment effects with level %.2f"), AbandonmentLevel);

    // Add dust, cobwebs, scattered belongings, decay effects
    // This would spawn appropriate abandonment props and modify existing ones
}

TArray<FVector> AInteriorDesignSystem::FindPropPlacementLocations(const FInteriorZoneLayout& Zone, int32 MaxProps)
{
    TArray<FVector> ValidLocations;

    int32 AttemptCount = 0;
    const int32 MaxAttempts = MaxProps * 5;

    while (ValidLocations.Num() < MaxProps && AttemptCount < MaxAttempts)
    {
        AttemptCount++;

        // Generate random location within zone bounds
        FVector RandomLocation = Zone.ZoneCenter + FVector(
            FMath::RandRange(-Zone.ZoneBounds.X * 0.4f, Zone.ZoneBounds.X * 0.4f),
            FMath::RandRange(-Zone.ZoneBounds.Y * 0.4f, Zone.ZoneBounds.Y * 0.4f),
            0.0f
        );

        // Check if location is suitable (not too close to other props)
        bool bLocationValid = true;
        for (const FVector& ExistingLocation : ValidLocations)
        {
            if (FVector::Dist2D(RandomLocation, ExistingLocation) < 100.0f) // 1m minimum distance
            {
                bLocationValid = false;
                break;
            }
        }

        if (bLocationValid)
        {
            ValidLocations.Add(RandomLocation);
        }
    }

    return ValidLocations;
}

UStaticMesh* AInteriorDesignSystem::SelectAppropriateProps(EInteriorPropType PropType, const FString& Context)
{
    // In a real implementation, this would select from the prop mesh library
    // based on the prop type and context
    
    TArray<UStaticMesh*>* PropMeshes = PropMeshLibrary.Find(PropType);
    if (PropMeshes && PropMeshes->Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, PropMeshes->Num() - 1);
        return (*PropMeshes)[RandomIndex];
    }

    return nullptr;
}

void AInteriorDesignSystem::InitializePropLibrary()
{
    UE_LOG(LogTemp, Warning, TEXT("InteriorDesignSystem: Initializing prop mesh library"));

    // Initialize prop mesh library
    PropMeshLibrary.Empty();

    // In a real implementation, these would load actual mesh assets
    // PropMeshLibrary.Add(EInteriorPropType::Furniture, {LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Props/SM_StoneStool"))});

    UE_LOG(LogTemp, Log, TEXT("InteriorDesignSystem: Prop library initialized"));
}

void AInteriorDesignSystem::CreateFirePitArea(const FVector& Location)
{
    if (!GetWorld())
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("InteriorDesignSystem: Creating fire pit area at %s"), *Location.ToString());

    // Create fire pit with surrounding stones and ash area
    // This would spawn appropriate fire pit props and charred ground effects
}

void AInteriorDesignSystem::CreateSleepingArea(const FVector& Location, int32 NumOccupants)
{
    if (!GetWorld())
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("InteriorDesignSystem: Creating sleeping area for %d occupants at %s"), 
           NumOccupants, *Location.ToString());

    // Create sleeping arrangements based on number of occupants
    // This would spawn bedding, personal storage, and comfort items
}

void AInteriorDesignSystem::CreateStorageArea(const FVector& Location, const TArray<FString>& StoredItems)
{
    if (!GetWorld())
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("InteriorDesignSystem: Creating storage area with %d item types at %s"), 
           StoredItems.Num(), *Location.ToString());

    // Create storage containers and organize stored items
    for (const FString& Item : StoredItems)
    {
        UE_LOG(LogTemp, Log, TEXT("InteriorDesignSystem: Storing item: %s"), *Item);
        // This would spawn appropriate storage props for each item type
    }
}

void AInteriorDesignSystem::CreateWorkArea(const FVector& Location, const FString& CraftType)
{
    if (!GetWorld())
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("InteriorDesignSystem: Creating work area for %s at %s"), 
           *CraftType, *Location.ToString());

    // Create work surface and tools appropriate for the craft type
    // This would spawn workbenches, tools, materials, and work-in-progress items
}

void AInteriorDesignSystem::AddWearAndTearEffects(AActor* PropActor, float WearLevel)
{
    if (!PropActor)
    {
        return;
    }

    // Apply visual wear effects to the prop based on wear level
    AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(PropActor);
    if (MeshActor && MeshActor->GetStaticMeshComponent())
    {
        UMaterialInstanceDynamic* DynamicMaterial = MeshActor->GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0);
        if (DynamicMaterial)
        {
            DynamicMaterial->SetScalarParameterValue(FName("WearLevel"), WearLevel);
            DynamicMaterial->SetScalarParameterValue(FName("Roughness"), 0.7f + (WearLevel * 0.3f));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("InteriorDesignSystem: Applied wear level %.2f to prop"), WearLevel);
}

void AInteriorDesignSystem::CreatePersonalTouches(const FVector& Location, const FString& PersonalityTrait)
{
    if (!GetWorld())
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("InteriorDesignSystem: Adding personal touch '%s' at %s"), 
           *PersonalityTrait, *Location.ToString());

    // Add personality-specific details based on the trait
    // This would spawn appropriate props that reflect the inhabitant's personality
}

void AInteriorDesignSystem::AddEnvironmentalClues(const FVector& Location, const FString& Clue)
{
    if (!GetWorld())
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("InteriorDesignSystem: Adding environmental clue '%s' at %s"), 
           *Clue, *Location.ToString());

    // Add visual clues that tell the story
    // This would create subtle environmental details that players can discover and interpret
}