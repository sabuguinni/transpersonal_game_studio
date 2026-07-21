#include "Arch_InteriorLayout.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

AArch_InteriorLayout::AArch_InteriorLayout()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 5.0f;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    StructureRadius = 400.0f;
    MaxItems = 12;
    bAutoGenerateLayout = true;
}

void AArch_InteriorLayout::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoGenerateLayout && InteriorItems.Num() == 0)
    {
        GeneratePrehistoricLayout();
    }
}

void AArch_InteriorLayout::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateItemConditions(DeltaTime);
}

void AArch_InteriorLayout::GeneratePrehistoricLayout()
{
    ClearLayout();
    
    // Essential items for prehistoric dwelling
    FVector Center = GetActorLocation();
    
    // Fire pit at center (essential)
    PlaceFirePit(Center);
    
    // Sleeping areas around the walls
    float AngleStep = 360.0f / 4.0f;
    for (int32 i = 0; i < 3; i++)
    {
        float Angle = FMath::DegreesToRadians(i * AngleStep + 45.0f);
        FVector SleepLocation = Center + FVector(
            FMath::Cos(Angle) * (StructureRadius * 0.7f),
            FMath::Sin(Angle) * (StructureRadius * 0.7f),
            0.0f
        );
        PlaceSleepingArea(SleepLocation);
    }
    
    // Tool storage near entrance
    FVector ToolLocation = Center + FVector(0, -StructureRadius * 0.6f, 0);
    PlaceToolStorage(ToolLocation);
    
    // Food storage opposite to entrance
    FVector FoodLocation = Center + FVector(0, StructureRadius * 0.6f, 0);
    PlaceFoodStorage(FoodLocation);
    
    // Additional items randomly placed
    int32 AdditionalItems = FMath::Min(4, MaxItems - InteriorItems.Num());
    for (int32 i = 0; i < AdditionalItems; i++)
    {
        float RandomAngle = FMath::RandRange(0.0f, 360.0f);
        float RandomRadius = FMath::RandRange(StructureRadius * 0.3f, StructureRadius * 0.8f);
        
        FVector RandomLocation = Center + FVector(
            FMath::Cos(FMath::DegreesToRadians(RandomAngle)) * RandomRadius,
            FMath::Sin(FMath::DegreesToRadians(RandomAngle)) * RandomRadius,
            0.0f
        );
        
        FArch_InteriorItem RandomItem;
        RandomItem.ItemName = FString::Printf(TEXT("Misc_Item_%d"), i);
        RandomItem.RelativeLocation = RandomLocation - Center;
        RandomItem.RelativeRotation = FRotator(0, RandomAngle, 0);
        RandomItem.ItemCondition = FMath::RandRange(60.0f, 100.0f);
        RandomItem.bIsEssential = false;
        
        InteriorItems.Add(RandomItem);
    }
}

void AArch_InteriorLayout::PlaceFirePit(const FVector& Location)
{
    FArch_InteriorItem FirePit;
    FirePit.ItemName = TEXT("FirePit_Central");
    FirePit.RelativeLocation = Location - GetActorLocation();
    FirePit.RelativeRotation = FRotator::ZeroRotator;
    FirePit.ItemCondition = 100.0f;
    FirePit.bIsEssential = true;
    
    InteriorItems.Add(FirePit);
    
    // Create visual representation
    UStaticMeshComponent* FirePitMesh = CreateDefaultSubobject<UStaticMeshComponent>(
        *FString::Printf(TEXT("FirePit_%d"), SpawnedItemMeshes.Num())
    );
    if (FirePitMesh)
    {
        FirePitMesh->SetupAttachment(RootComponent);
        FirePitMesh->SetRelativeLocation(FirePit.RelativeLocation);
        FirePitMesh->SetRelativeRotation(FirePit.RelativeRotation);
        SpawnedItemMeshes.Add(FirePitMesh);
    }
}

void AArch_InteriorLayout::PlaceSleepingArea(const FVector& Location)
{
    FArch_InteriorItem SleepingArea;
    SleepingArea.ItemName = FString::Printf(TEXT("SleepingArea_%d"), InteriorItems.Num());
    SleepingArea.RelativeLocation = Location - GetActorLocation();
    SleepingArea.RelativeRotation = FRotator(0, FMath::RandRange(-30.0f, 30.0f), 0);
    SleepingArea.ItemCondition = FMath::RandRange(70.0f, 95.0f);
    SleepingArea.bIsEssential = true;
    
    InteriorItems.Add(SleepingArea);
    
    // Create visual representation
    UStaticMeshComponent* BedMesh = CreateDefaultSubobject<UStaticMeshComponent>(
        *FString::Printf(TEXT("Bed_%d"), SpawnedItemMeshes.Num())
    );
    if (BedMesh)
    {
        BedMesh->SetupAttachment(RootComponent);
        BedMesh->SetRelativeLocation(SleepingArea.RelativeLocation);
        BedMesh->SetRelativeRotation(SleepingArea.RelativeRotation);
        SpawnedItemMeshes.Add(BedMesh);
    }
}

void AArch_InteriorLayout::PlaceToolStorage(const FVector& Location)
{
    FArch_InteriorItem ToolStorage;
    ToolStorage.ItemName = TEXT("ToolStorage_Main");
    ToolStorage.RelativeLocation = Location - GetActorLocation();
    ToolStorage.RelativeRotation = FRotator(0, 180.0f, 0);
    ToolStorage.ItemCondition = 85.0f;
    ToolStorage.bIsEssential = true;
    
    InteriorItems.Add(ToolStorage);
    
    // Create visual representation
    UStaticMeshComponent* ToolMesh = CreateDefaultSubobject<UStaticMeshComponent>(
        *FString::Printf(TEXT("Tools_%d"), SpawnedItemMeshes.Num())
    );
    if (ToolMesh)
    {
        ToolMesh->SetupAttachment(RootComponent);
        ToolMesh->SetRelativeLocation(ToolStorage.RelativeLocation);
        ToolMesh->SetRelativeRotation(ToolStorage.RelativeRotation);
        SpawnedItemMeshes.Add(ToolMesh);
    }
}

void AArch_InteriorLayout::PlaceFoodStorage(const FVector& Location)
{
    FArch_InteriorItem FoodStorage;
    FoodStorage.ItemName = TEXT("FoodStorage_Main");
    FoodStorage.RelativeLocation = Location - GetActorLocation();
    FoodStorage.RelativeRotation = FRotator(0, 0.0f, 0);
    FoodStorage.ItemCondition = 75.0f;
    FoodStorage.bIsEssential = true;
    
    InteriorItems.Add(FoodStorage);
    
    // Create visual representation
    UStaticMeshComponent* FoodMesh = CreateDefaultSubobject<UStaticMeshComponent>(
        *FString::Printf(TEXT("Food_%d"), SpawnedItemMeshes.Num())
    );
    if (FoodMesh)
    {
        FoodMesh->SetupAttachment(RootComponent);
        FoodMesh->SetRelativeLocation(FoodStorage.RelativeLocation);
        FoodMesh->SetRelativeRotation(FoodStorage.RelativeRotation);
        SpawnedItemMeshes.Add(FoodMesh);
    }
}

void AArch_InteriorLayout::UpdateItemConditions(float DeltaTime)
{
    for (FArch_InteriorItem& Item : InteriorItems)
    {
        // Gradual degradation over time
        float DegradationRate = Item.bIsEssential ? 0.01f : 0.02f;
        Item.ItemCondition = FMath::Max(0.0f, Item.ItemCondition - (DegradationRate * DeltaTime));
        
        // Remove completely degraded non-essential items
        if (!Item.bIsEssential && Item.ItemCondition <= 0.0f)
        {
            // Mark for removal (would need additional logic to actually remove)
        }
    }
}

void AArch_InteriorLayout::ClearLayout()
{
    InteriorItems.Empty();
    
    // Destroy spawned mesh components
    for (UStaticMeshComponent* MeshComp : SpawnedItemMeshes)
    {
        if (MeshComp)
        {
            MeshComp->DestroyComponent();
        }
    }
    SpawnedItemMeshes.Empty();
}

bool AArch_InteriorLayout::HasEssentialItems() const
{
    int32 EssentialCount = 0;
    for (const FArch_InteriorItem& Item : InteriorItems)
    {
        if (Item.bIsEssential && Item.ItemCondition > 10.0f)
        {
            EssentialCount++;
        }
    }
    
    return EssentialCount >= 3; // Fire pit, sleeping area, and storage
}