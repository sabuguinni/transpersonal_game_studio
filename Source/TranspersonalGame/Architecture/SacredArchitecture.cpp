// SacredArchitecture.cpp
// Implementação do sistema de arquitetura sagrada

#include "SacredArchitecture.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

ASacredArchitecture::ASacredArchitecture()
{
    PrimaryActorTick.bCanEverTick = true;

    // Componente raiz
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    // Estrutura principal
    MainStructure = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainStructure"));
    MainStructure->SetupAttachment(RootComponent);

    // Valores padrão
    ArchitectureType = ESacredArchitectureType::Temple;
    Parameters.Scale = 1.0f;
    Parameters.Height = 10.0f;
    Parameters.Segments = 8;
    Parameters.GoldenRatio = 1.618f;
    Parameters.bUsePhiProportions = true;
    Parameters.EnergyIntensity = 1.0f;

    CurrentEnergyLevel = 0.0f;
    TimeAccumulator = 0.0f;
    bEnergyFieldActive = false;
}

void ASacredArchitecture::BeginPlay()
{
    Super::BeginPlay();
    
    GenerateStructure();
    SetupEnergyResonance();
}

void ASacredArchitecture::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeAccumulator += DeltaTime;

    if (bEnergyFieldActive)
    {
        // Pulsação energética baseada em frequências sagradas
        float SacredFrequency = 432.0f; // Hz
        float PulseValue = FMath::Sin(TimeAccumulator * SacredFrequency * 0.01f);
        CurrentEnergyLevel = (PulseValue + 1.0f) * 0.5f * Parameters.EnergyIntensity;

        // Atualizar materiais com energia
        if (MainStructure && EnergyMaterial)
        {
            UMaterialInstanceDynamic* DynamicMaterial = MainStructure->CreateAndSetMaterialInstanceDynamic(0);
            if (DynamicMaterial)
            {
                DynamicMaterial->SetScalarParameterValue(TEXT("EnergyLevel"), CurrentEnergyLevel);
                DynamicMaterial->SetScalarParameterValue(TEXT("TimeParam"), TimeAccumulator);
            }
        }
    }
}

void ASacredArchitecture::GenerateStructure()
{
    switch (ArchitectureType)
    {
        case ESacredArchitectureType::Temple:
            GenerateTemple();
            break;
        case ESacredArchitectureType::Pyramid:
            GeneratePyramid();
            break;
        case ESacredArchitectureType::Mandala:
            GenerateMandalaStructure();
            break;
        case ESacredArchitectureType::Spiral:
            GenerateSpiralTower();
            break;
        case ESacredArchitectureType::Crystal:
            GenerateCrystalFormation();
            break;
        case ESacredArchitectureType::Labyrinth:
            GenerateLabyrinth();
            break;
        case ESacredArchitectureType::Portal:
            GeneratePortalGateway();
            break;
        case ESacredArchitectureType::Sanctuary:
            GenerateSanctuary();
            break;
    }

    if (Parameters.bUsePhiProportions)
    {
        ApplyGoldenRatioProportions();
    }

    OnStructureGenerated();
}

void ASacredArchitecture::GenerateTemple()
{
    // Templo baseado em proporções sagradas
    FVector BaseScale = FVector(Parameters.Scale, Parameters.Scale, Parameters.Height);
    
    if (Parameters.bUsePhiProportions)
    {
        BaseScale.X *= Parameters.GoldenRatio;
        BaseScale.Y *= Parameters.GoldenRatio;
    }

    MainStructure->SetWorldScale3D(BaseScale);
    
    // Aplicar rotação baseada em geometria sagrada
    FRotator SacredRotation = FRotator(0.0f, 360.0f / Parameters.Segments, 0.0f);
    MainStructure->SetWorldRotation(SacredRotation);
}

void ASacredArchitecture::GeneratePyramid()
{
    // Pirâmide com ângulos baseados na Grande Pirâmide
    float PyramidAngle = 51.84f; // Ângulo da Grande Pirâmide
    FVector PyramidScale = FVector(Parameters.Scale, Parameters.Scale, Parameters.Height);
    
    MainStructure->SetWorldScale3D(PyramidScale);
    MainStructure->SetWorldRotation(FRotator(0.0f, PyramidAngle, 0.0f));
}

void ASacredArchitecture::GenerateMandalaStructure()
{
    // Estrutura mandala com múltiplos segmentos
    FVector MandalaScale = FVector(Parameters.Scale, Parameters.Scale, Parameters.Height * 0.5f);
    MainStructure->SetWorldScale3D(MandalaScale);
    
    // Rotação baseada em divisões sagradas
    float RotationStep = 360.0f / Parameters.Segments;
    MainStructure->SetWorldRotation(FRotator(0.0f, RotationStep, 0.0f));
}

void ASacredArchitecture::GenerateSpiralTower()
{
    // Torre espiral baseada na sequência de Fibonacci
    FVector SpiralScale = FVector(Parameters.Scale, Parameters.Scale, Parameters.Height * Parameters.GoldenRatio);
    MainStructure->SetWorldScale3D(SpiralScale);
    
    // Rotação espiral
    float SpiralRotation = Parameters.GoldenRatio * 137.5f; // Ângulo áureo
    MainStructure->SetWorldRotation(FRotator(0.0f, SpiralRotation, 0.0f));
}

void ASacredArchitecture::GenerateCrystalFormation()
{
    // Formação cristalina com frequências harmônicas
    FVector CrystalScale = FVector(Parameters.Scale, Parameters.Scale, Parameters.Height);
    
    // Aplicar proporções cristalinas
    CrystalScale *= FMath::Pow(Parameters.GoldenRatio, 0.5f);
    MainStructure->SetWorldScale3D(CrystalScale);
}

void ASacredArchitecture::GenerateLabyrinth()
{
    // Labirinto baseado em padrões sagrados
    FVector LabyrinthScale = FVector(Parameters.Scale * 2.0f, Parameters.Scale * 2.0f, Parameters.Height * 0.3f);
    MainStructure->SetWorldScale3D(LabyrinthScale);
}

void ASacredArchitecture::GeneratePortalGateway()
{
    // Portal com dimensões baseadas em proporções cósmicas
    FVector PortalScale = FVector(Parameters.Scale, Parameters.Scale * 0.2f, Parameters.Height * Parameters.GoldenRatio);
    MainStructure->SetWorldScale3D(PortalScale);
}

void ASacredArchitecture::GenerateSanctuary()
{
    // Santuário com proporções harmônicas
    FVector SanctuaryScale = FVector(Parameters.Scale * Parameters.GoldenRatio, Parameters.Scale * Parameters.GoldenRatio, Parameters.Height);
    MainStructure->SetWorldScale3D(SanctuaryScale);
}

void ASacredArchitecture::SetArchitectureType(ESacredArchitectureType NewType)
{
    ArchitectureType = NewType;
    GenerateStructure();
}

void ASacredArchitecture::UpdateParameters(const FArchitecturalParameters& NewParams)
{
    Parameters = NewParams;
    GenerateStructure();
}

void ASacredArchitecture::ActivateEnergyField()
{
    bEnergyFieldActive = true;
    
    if (EnergyField)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EnergyField, GetActorLocation());
    }
    
    if (AmbientResonance)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), AmbientResonance, GetActorLocation());
    }
    
    OnEnergyFieldActivated();
}

void ASacredArchitecture::DeactivateEnergyField()
{
    bEnergyFieldActive = false;
    CurrentEnergyLevel = 0.0f;
}

void ASacredArchitecture::ApplyGoldenRatioProportions()
{
    FVector CurrentScale = MainStructure->GetComponentScale();
    
    // Aplicar proporção áurea
    CurrentScale.X *= Parameters.GoldenRatio;
    CurrentScale.Z *= FMath::Pow(Parameters.GoldenRatio, 0.5f);
    
    MainStructure->SetWorldScale3D(CurrentScale);
}

void ASacredArchitecture::SetupEnergyResonance()
{
    // Configurar ressonância energética baseada em frequências sagradas
    CurrentEnergyLevel = 0.0f;
    TimeAccumulator = 0.0f;
}