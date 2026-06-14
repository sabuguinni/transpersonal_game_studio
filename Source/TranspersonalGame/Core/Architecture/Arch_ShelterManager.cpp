#include "Arch_ShelterManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"

AArch_ShelterManager::AArch_ShelterManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create entrance mesh component
    EntranceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EntranceMesh"));
    RootComponent = EntranceMesh;

    // Create interior trigger volume
    InteriorTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("InteriorTrigger"));
    InteriorTrigger->SetupAttachment(RootComponent);
    InteriorTrigger->SetBoxExtent(FVector(200, 200, 150));
    InteriorTrigger->SetRelativeLocation(FVector(100, 0, 0));

    // Create fire pit mesh
    FirePitMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FirePitMesh"));
    FirePitMesh->SetupAttachment(RootComponent);
    FirePitMesh->SetRelativeLocation(FVector(120, 20, -50));
    FirePitMesh->SetVisibility(false);

    // Initialize default values
    bPlayerInside = false;
    CurrentProtection = 0.0f;

    // Set default shelter data
    ShelterData.ShelterType = EArch_ShelterType::StoneArchway;
    ShelterData.bHasFirePit = false;
    ShelterData.bHasSleepingArea = true;
    ShelterData.ProtectionLevel = 0.7f;
}

void AArch_ShelterManager::BeginPlay()
{
    Super::BeginPlay();

    // Setup trigger events
    if (InteriorTrigger)
    {
        InteriorTrigger->OnComponentBeginOverlap.AddDynamic(this, &AArch_ShelterManager::OnInteriorEntered);
        InteriorTrigger->OnComponentEndOverlap.AddDynamic(this, &AArch_ShelterManager::OnInteriorExited);
    }

    // Initialize shelter based on configuration
    UpdateMeshBasedOnType();
    SetupInteriorTrigger();
}

void AArch_ShelterManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update protection level based on player presence
    if (bPlayerInside)
    {
        CurrentProtection = ShelterData.ProtectionLevel;
        
        // Additional protection from fire pit
        if (ShelterData.bHasFirePit && FirePitMesh->IsVisible())
        {
            CurrentProtection += 0.2f;
        }
    }
    else
    {
        CurrentProtection = 0.0f;
    }
}

void AArch_ShelterManager::InitializeShelter(const FArch_ShelterData& NewShelterData)
{
    ShelterData = NewShelterData;
    UpdateMeshBasedOnType();
    SetupInteriorTrigger();

    // Enable/disable fire pit based on configuration
    ToggleFirePit(ShelterData.bHasFirePit);
}

void AArch_ShelterManager::SetShelterType(EArch_ShelterType NewType)
{
    ShelterData.ShelterType = NewType;
    UpdateMeshBasedOnType();
}

bool AArch_ShelterManager::IsPlayerInShelter() const
{
    return bPlayerInside;
}

float AArch_ShelterManager::GetProtectionLevel() const
{
    return CurrentProtection;
}

void AArch_ShelterManager::ToggleFirePit(bool bEnable)
{
    ShelterData.bHasFirePit = bEnable;
    if (FirePitMesh)
    {
        FirePitMesh->SetVisibility(bEnable);
    }
}

void AArch_ShelterManager::OnInteriorEntered(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        bPlayerInside = true;
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                FString::Printf(TEXT("Entered shelter - Protection: %.1f"), ShelterData.ProtectionLevel));
        }
    }
}

void AArch_ShelterManager::OnInteriorExited(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        bPlayerInside = false;
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Left shelter"));
        }
    }
}

void AArch_ShelterManager::UpdateMeshBasedOnType()
{
    // Update mesh based on shelter type
    // This would normally load different static meshes based on type
    // For now, we'll just adjust the scale and position
    
    switch (ShelterData.ShelterType)
    {
        case EArch_ShelterType::StoneArchway:
            if (EntranceMesh)
            {
                EntranceMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.2f));
            }
            break;
            
        case EArch_ShelterType::CaveEntrance:
            if (EntranceMesh)
            {
                EntranceMesh->SetRelativeScale3D(FVector(1.5f, 1.5f, 0.8f));
            }
            break;
            
        case EArch_ShelterType::RockOverhang:
            if (EntranceMesh)
            {
                EntranceMesh->SetRelativeScale3D(FVector(2.0f, 1.0f, 0.6f));
            }
            break;
            
        case EArch_ShelterType::LogShelter:
            if (EntranceMesh)
            {
                EntranceMesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 0.8f));
            }
            break;
            
        case EArch_ShelterType::BuriedDwelling:
            if (EntranceMesh)
            {
                EntranceMesh->SetRelativeScale3D(FVector(0.6f, 0.6f, 0.4f));
            }
            break;
    }
}

void AArch_ShelterManager::SetupInteriorTrigger()
{
    if (InteriorTrigger)
    {
        // Adjust trigger volume based on interior bounds
        FVector TriggerExtent = ShelterData.InteriorBounds * 0.5f;
        InteriorTrigger->SetBoxExtent(TriggerExtent);
        
        // Position trigger slightly inside the entrance
        FVector TriggerOffset = FVector(TriggerExtent.X, 0, 0);
        InteriorTrigger->SetRelativeLocation(TriggerOffset);
    }
}