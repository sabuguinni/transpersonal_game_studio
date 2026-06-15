#include "EnvArt_WeatheredBoulder.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"

AEnvArt_WeatheredBoulder::AEnvArt_WeatheredBoulder()
{
    PrimaryActorTick.bCanEverTick = false;

    // Initialize default values
    BoulderType = EEnvArt_BoulderType::Granite;
    WeatheringLevel = EEnvArt_WeatheringLevel::ModerateWeathering;
    SizeScale = 1.0f;
    bHasMossGrowth = true;
    bHasLichenGrowth = false;
    bCanBeClimbed = true;
    bProvidesShade = true;
    bHasToolMarks = false;
    bHasAnimalScratchMarks = false;
    bNearWaterSource = false;
    StoryContext = TEXT("Ancient boulder weathered by millennia of wind and rain");

    // Setup climbing trigger
    ClimbingTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("ClimbingTrigger"));
    ClimbingTrigger->SetupAttachment(RootComponent);
    ClimbingTrigger->SetBoxExtent(FVector(200.0f, 200.0f, 300.0f));
    ClimbingTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ClimbingTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    ClimbingTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Setup shade trigger
    ShadeTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("ShadeTrigger"));
    ShadeTrigger->SetupAttachment(RootComponent);
    ShadeTrigger->SetBoxExtent(FVector(400.0f, 400.0f, 100.0f));
    ShadeTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ShadeTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    ShadeTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Initialize arrays
    BoulderMaterials.Empty();
    MossMaterials.Empty();
    LichenMaterials.Empty();
}

void AEnvArt_WeatheredBoulder::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (ClimbingTrigger)
    {
        ClimbingTrigger->OnComponentBeginOverlap.AddDynamic(this, &AEnvArt_WeatheredBoulder::OnClimbingTriggerBeginOverlap);
    }

    if (ShadeTrigger)
    {
        ShadeTrigger->OnComponentBeginOverlap.AddDynamic(this, &AEnvArt_WeatheredBoulder::OnShadeTriggerBeginOverlap);
        ShadeTrigger->OnComponentEndOverlap.AddDynamic(this, &AEnvArt_WeatheredBoulder::OnShadeTriggerEndOverlap);
    }

    // Initialize boulder appearance
    InitializeBoulderMesh();
    ApplyWeatheringEffects();
    SetupCollisionAndTriggers();
}

void AEnvArt_WeatheredBoulder::ApplyWeatheringEffects()
{
    if (!GetStaticMeshComponent())
    {
        return;
    }

    // Apply size scaling based on weathering
    FVector CurrentScale = GetActorScale3D();
    float WeatheringScaleFactor = 1.0f;

    switch (WeatheringLevel)
    {
    case EEnvArt_WeatheringLevel::Fresh:
        WeatheringScaleFactor = 1.0f;
        break;
    case EEnvArt_WeatheringLevel::LightWeathering:
        WeatheringScaleFactor = 0.95f;
        break;
    case EEnvArt_WeatheringLevel::ModerateWeathering:
        WeatheringScaleFactor = 0.9f;
        break;
    case EEnvArt_WeatheringLevel::HeavyWeathering:
        WeatheringScaleFactor = 0.85f;
        break;
    case EEnvArt_WeatheringLevel::Ancient:
        WeatheringScaleFactor = 0.8f;
        break;
    }

    SetActorScale3D(CurrentScale * SizeScale * WeatheringScaleFactor);

    // Update materials based on properties
    UpdateMaterialBasedOnProperties();
}

void AEnvArt_WeatheredBoulder::SetBoulderType(EEnvArt_BoulderType NewType)
{
    BoulderType = NewType;
    UpdateMaterialBasedOnProperties();
}

void AEnvArt_WeatheredBoulder::SetWeatheringLevel(EEnvArt_WeatheringLevel NewLevel)
{
    WeatheringLevel = NewLevel;
    ApplyWeatheringEffects();
}

void AEnvArt_WeatheredBoulder::ToggleMossGrowth(bool bEnable)
{
    bHasMossGrowth = bEnable;
    UpdateMaterialBasedOnProperties();
}

void AEnvArt_WeatheredBoulder::ToggleLichenGrowth(bool bEnable)
{
    bHasLichenGrowth = bEnable;
    UpdateMaterialBasedOnProperties();
}

FVector AEnvArt_WeatheredBoulder::GetClimbingPosition() const
{
    if (bCanBeClimbed)
    {
        FVector ActorLocation = GetActorLocation();
        FVector ActorBounds = GetStaticMeshComponent()->Bounds.BoxExtent;
        return ActorLocation + FVector(0.0f, 0.0f, ActorBounds.Z * 0.8f);
    }
    return GetActorLocation();
}

bool AEnvArt_WeatheredBoulder::CanProvideShade() const
{
    return bProvidesShade && SizeScale >= 1.5f;
}

void AEnvArt_WeatheredBoulder::OnClimbingTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (bCanBeClimbed && OtherActor && OtherActor->IsA<ACharacter>())
    {
        OnPlayerClimbed();
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                FString::Printf(TEXT("Player can climb this %s boulder"), 
                *UEnum::GetValueAsString(BoulderType)));
        }
    }
}

void AEnvArt_WeatheredBoulder::OnShadeTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (CanProvideShade() && OtherActor && OtherActor->IsA<ACharacter>())
    {
        OnPlayerEnteredShade();
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, 
                TEXT("Entered boulder shade - temperature relief"));
        }
    }
}

void AEnvArt_WeatheredBoulder::OnShadeTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (CanProvideShade() && OtherActor && OtherActor->IsA<ACharacter>())
    {
        OnPlayerExitedShade();
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, 
                TEXT("Left boulder shade"));
        }
    }
}

void AEnvArt_WeatheredBoulder::InitializeBoulderMesh()
{
    if (GetStaticMeshComponent())
    {
        // Try to load a basic rock mesh - fallback to engine primitives if needed
        UStaticMesh* RockMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"));
        if (RockMesh)
        {
            GetStaticMeshComponent()->SetStaticMesh(RockMesh);
        }

        // Set collision properties
        GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        GetStaticMeshComponent()->SetCollisionResponseToAllChannels(ECR_Block);
        GetStaticMeshComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    }
}

void AEnvArt_WeatheredBoulder::UpdateMaterialBasedOnProperties()
{
    if (!GetStaticMeshComponent())
    {
        return;
    }

    // Apply base boulder material based on type
    int32 MaterialIndex = static_cast<int32>(BoulderType);
    if (BoulderMaterials.IsValidIndex(MaterialIndex) && BoulderMaterials[MaterialIndex])
    {
        GetStaticMeshComponent()->SetMaterial(0, BoulderMaterials[MaterialIndex]);
    }

    // Apply moss overlay if enabled
    if (bHasMossGrowth && MossMaterials.Num() > 0)
    {
        int32 MossIndex = static_cast<int32>(WeatheringLevel) % MossMaterials.Num();
        if (MossMaterials.IsValidIndex(MossIndex) && MossMaterials[MossIndex])
        {
            GetStaticMeshComponent()->SetMaterial(1, MossMaterials[MossIndex]);
        }
    }

    // Apply lichen overlay if enabled
    if (bHasLichenGrowth && LichenMaterials.Num() > 0)
    {
        int32 LichenIndex = FMath::RandRange(0, LichenMaterials.Num() - 1);
        if (LichenMaterials.IsValidIndex(LichenIndex) && LichenMaterials[LichenIndex])
        {
            GetStaticMeshComponent()->SetMaterial(2, LichenMaterials[LichenIndex]);
        }
    }
}

void AEnvArt_WeatheredBoulder::SetupCollisionAndTriggers()
{
    // Adjust trigger sizes based on boulder scale
    if (ClimbingTrigger)
    {
        FVector ClimbingExtent = FVector(200.0f, 200.0f, 300.0f) * SizeScale;
        ClimbingTrigger->SetBoxExtent(ClimbingExtent);
    }

    if (ShadeTrigger)
    {
        FVector ShadeExtent = FVector(400.0f, 400.0f, 100.0f) * SizeScale;
        ShadeTrigger->SetBoxExtent(ShadeExtent);
        
        // Position shade trigger slightly above ground
        FVector ShadeOffset = FVector(0.0f, 0.0f, -50.0f * SizeScale);
        ShadeTrigger->SetRelativeLocation(ShadeOffset);
    }
}